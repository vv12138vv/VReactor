#include "tcp_server.h"
#include "acceptor.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "net_address.h"
#include "tcp_connection.h"
#include <cstring>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>




TcpServer::TcpServer(EventLoop& base_loop, const NetAddress& listen_addr, const std::string& name)
    : base_loop_(base_loop)
    , listen_addr_(listen_addr)
    , name_(name)
    , accpetor_(std::make_unique<Acceptor>(base_loop_, listen_addr_))
    , thread_pool_(std::make_shared<EventLoopThreadPool>(base_loop_, name_))
    , connection_cb_()
    , read_cb_()
    , write_cb_()
    , thread_init_cb_()
    , started_cnt_(0)
    , next_connection_idx_(1) {
    accpetor_->set_cb(std::bind(&TcpServer::new_connection, this, std::placeholders::_1, std::placeholders::_2));
}


TcpServer::~TcpServer() {
    for (auto& it : connections_) {
        auto ptr = it.second;
        it.second.reset();
        ptr->get_loop().run_in_loop(std::bind(&TcpConnection::connection_destroyed, ptr));
    }
}

void TcpServer::start() {
    if (started_cnt_++ == 0) {
        thread_pool_->start(thread_init_cb_);
        base_loop_.run_in_loop(std::bind(&Acceptor::listen, accpetor_.get()));
    }
}


void TcpServer::new_connection(int sock_fd, const NetAddress& peer_addr) {
    //选择一个sub_loop来管理
    EventLoop* sub_loop = thread_pool_->next_loop();
    std::string connection_name = listen_addr_.get_ip_port() + " id:" + std::to_string(next_connection_idx_);
    next_connection_idx_ += 1;
    LOG_INFO << "TcpServer::new_connection [" << name_ << "] "
             << "create new connection: " << connection_name << " from " << peer_addr.get_ip_port();

    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addr_len = sizeof(local);
    if (::getsockname(sock_fd, (sockaddr*)&local, &addr_len) < 0) {
        LOG_ERROR << "get socket local address failed";
        return;
    }
    NetAddress local_addr(local);
    auto new_conn = std::make_shared<TcpConnection>(sub_loop, connection_name, sock_fd, local_addr, peer_addr);
    connections_[connection_name] = new_conn;

    //初始化connection的设置
    new_conn->set_connection_callback(connection_cb_);
    new_conn->set_read_callback(read_cb_);
    new_conn->set_write_callback(write_cb_);
    new_conn->set_close_callback(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
    sub_loop->run_in_loop(std::bind(&TcpConnection::connection_established, new_conn));
}

void TcpServer::remove_connection(const std::shared_ptr<TcpConnection>& connection) {
    base_loop_.run_in_loop(std::bind(&TcpServer::remove_connection_in_loop, this, connection));
}

void TcpServer::remove_connection_in_loop(const std::shared_ptr<TcpConnection>& connection) {
    connections_.erase(connection->get_name());
    LOG_INFO << "TcpServer remove connection in loop [" << name_ << ']';
    EventLoop& sub_loop = connection->get_loop();
    sub_loop.queue_in_loop(std::bind(&TcpConnection::connection_destroyed, connection));
}