#include "acceptor.h"
#include "logger.h"
#include "net_address.h"
#include "event_loop.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>



int creat_non_block_sock() {
    int sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock_fd == -1) {
        LOG_FATAL << "listen socket create err " << errno;
    }
    return sock_fd;
}


Acceptor::Acceptor(EventLoop& loop, const NetAddress& listen_addr)
    : loop_(loop)
    , accept_socket_(creat_non_block_sock())
    , accept_channel_(loop, accept_socket_.get_sock_fd())
    , listening_(false) {
    LOG_DEBUG << "Acceptor created fd:" << accept_socket_.get_sock_fd();

    accept_socket_.set_reuse_address(true);
    accept_socket_.set_reuse_port(true);
    accept_socket_.bind_address(listen_addr);

    accept_channel_.set_read_call_back(std::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
    accept_channel_.disable_all();
    accept_channel_.remove_from_loop();
}

void Acceptor::listen() {
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enable_read();
}

void Acceptor::handle_read() {
    NetAddress peer_addr;
    int connect_fd = accept_socket_.accept(&peer_addr);
    if (connect_fd >= 0) {
        if (new_connection_cb_ != nullptr) {
            new_connection_cb_(connect_fd, peer_addr);
        } else {
            LOG_DEBUG << "No new connection callback";
            ::close(connect_fd);
        }
    } else {
        LOG_ERROR << "accecpt failed";
        if (errno == EMFILE) {
            LOG_ERROR << "sockfd has reached limit";
        }
    }
}