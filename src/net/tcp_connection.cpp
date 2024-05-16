#include "tcp_connection.h"
#include "channel.h"
#include "logger.h"
#include "net_address.h"
#include "reactor.h"
#include "socket.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TcpConnection::TcpConnection(Reactor& loop, const std::string& name, int sock_fd, const NetAddress& local_addr, const NetAddress& peer_addr)
    : loop_(loop)
    , name_(name)
    , state_(State::Connecting)
    , reading_(true)
    , socket_(std::make_unique<Socket>(sock_fd))
    , channel_(std::make_unique<Channel>(loop, sock_fd))
    , local_addr_(local_addr)
    , peer_addr_(peer_addr)
    , send_limit_(64 * 1024 * 1024) {

    channel_->set_read_call_back(std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
    channel_->set_write_call_back(std::bind(&TcpConnection::handle_write, this));
    channel_->set_close_call_back(std::bind(&TcpConnection::handle_close, this));
    channel_->set_error_call_back(std::bind(&TcpConnection::handle_error, this));

    LOG_INFO << "TcpConnection: " << name_ << " created at fd: " << sock_fd;
    socket_->set_keep_alive(true);
}


TcpConnection::~TcpConnection() {
    LOG_INFO << "TcpConnection: " << name_ << " destructed at fd: " << socket_->get_sock_fd();
}


void TcpConnection::send(const std::string& data) {
    if (state_ == State::Connected) {
        if (loop_.is_in_loop_thread()) {
            send_in_loop(data.c_str(), data.size());
        } else {
            void (TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::send_in_loop;
            loop_.run_in_loop(std::bind(fp, this, data.c_str(), data.size()));
        }
    }
}
void TcpConnection::send(Buffer* buffer) {
    if (state_ == State::Connected) {
        if (loop_.is_in_loop_thread()) {
            send_in_loop(buffer->peek(), buffer->readable_size());
            buffer->retrieve_all();
        } else {
            void (TcpConnection::*fp)(const std::string& data) = &TcpConnection::send_in_loop;
            loop_.run_in_loop(std::bind(fp, this, buffer->retrieve_all_str()));
        }
    }
}
void TcpConnection::send_in_loop(const std::string& data) {
    send_in_loop(data.data(), data.size());
}

void TcpConnection::send_in_loop(const void* data, size_t len) {
    if (state_ == State::Disconnected) {
        LOG_ERROR << "Connection " << name_ << " has disconnected,give up writing";
        return;
    }
    ssize_t written = 0;
    size_t remain = len;
    bool is_error = false;
    // channel第一次写数据，且缓冲区无待发送数据
    if (channel_->is_writing() == false && output_buffer_.readable_size() == 0) {
        written = ::write(channel_->get_fd(), data, len);
        if (written >= 0) {
            remain = len - written;
            if (remain == 0 && write_cb_ != nullptr) {
                loop_.queue_in_loop(std::bind(write_cb_, shared_from_this()));
            }
        } else {
            written = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::send_in_loop error";
                if (errno == EPIPE || errno == ECONNRESET) {
                    is_error = true;
                }
            }
        }
    }
    if (is_error && remain > 0) {
        size_t old_len = output_buffer_.readable_size();
        if (old_len + remain >= send_limit_ && old_len < send_limit_ && over_limit_cb_ != nullptr) {
            loop_.queue_in_loop(std::bind(over_limit_cb_, shared_from_this(), old_len + remain));
        }
        output_buffer_.append(static_cast<const char*>(data) + written, remain);
        if (!channel_->is_writing()) {
            channel_->enable_read();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == State::Connected) {
        set_state(State::Disconnecting);
        loop_.run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, this));
    }
}

void TcpConnection::shutdown_in_loop() {
    if (!channel_->is_writing()) {
        socket_->shut_down_write();
    }
}


void TcpConnection::connection_established() {
    set_state(State::Connected);
    channel_->tie(shared_from_this());
    channel_->enable_read();

    connection_cb_(shared_from_this());
}

void TcpConnection::connection_destroyed() {
    if (state_ == State::Connected) {
        set_state(State::Disconnected);
        channel_->disable_all();
        close_cb_(shared_from_this());
    }
    channel_->remove_from_loop();
}


void TcpConnection::handle_read(TimePoint receive_time) {
    int save_errno = 0;
    ssize_t n = input_buffer_.read_fd(channel_->get_fd(), &save_errno);
    if (n > 0) {
        read_cb_(shared_from_this(), &input_buffer_, receive_time);
    } else if (n == 0) {
        handle_close();
    } else {
        errno = save_errno;
        LOG_ERROR << "TcpConnection handle_read failed";
        handle_error();
    }
}

void TcpConnection::handle_write() {
    if (channel_->is_writing()) {
        int save_errno = 0;
        ssize_t n = output_buffer_.write_fd(channel_->get_fd(), &save_errno);
        if (n > 0) {
            output_buffer_.retrieve(n);
            if (output_buffer_.readable_size() == 0) {
                channel_->disable_write();
                if (write_cb_ != nullptr) {
                    loop_.queue_in_loop(std::bind(write_cb_, shared_from_this()));
                }
                if (state_ == State::Disconnecting) {
                    shutdown_in_loop();
                }
            }
        } else {
            LOG_ERROR << "TcpConnection handle_write() failed";
        }
    } else {
        LOG_ERROR << "TcpConnection fd: " << channel_->get_fd() << " is down,no more writing";
    }
}

void TcpConnection::handle_close() {
    set_state(State::Disconnected);
    channel_->disable_all();
    close_cb_(shared_from_this());
}

void TcpConnection::handle_error() {
    int op = 0;
    socklen_t op_len = sizeof(op);
    int save_errno = 0;
    if (::getsockopt(channel_->get_fd(), SOL_SOCKET, SO_ERROR, &op, &op_len)) {
        save_errno = errno;
    } else {
        save_errno = op;
    }
    LOG_ERROR << "Connection handle_error name:" << name_ << " - SO_ERROR:" << save_errno;
}