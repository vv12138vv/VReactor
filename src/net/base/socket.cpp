#include "socket.h"
#include "logger.h"
#include "net_address.h"
#include <asm-generic/socket.h>
#include <cstring>
#include <strings.h>
#include <sys/socket.h>


Socket::~Socket() {
    close(sock_fd_);
}

void Socket::bind_address(const NetAddress& local_addr) {
    int ret = ::bind(sock_fd_, (sockaddr*)local_addr.get_sockaddr(), sizeof(sockaddr_in));
    if (ret != 0) {
        LOG_FATAL << "bind sock_fd:" << sock_fd_ << " failed";
    }
}

void Socket::listen() {
    int ret = ::listen(sock_fd_, SOMAXCONN);   //设置默认的连接队列长度
    if (ret != 0) {
        LOG_FATAL << "listen sock_fd: " << sock_fd_ << " failed";
    }
}

int Socket::accept(NetAddress* peer_addr) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ::memset(&addr, 0, sizeof(addr));
    int connection_fd = ::accept4(sock_fd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connection_fd >= 0) {
        peer_addr->set_address(addr);
    } else {
        LOG_ERROR << "accept4() failed";
    }
    return connection_fd;
}

void Socket::shut_down_write() {
    int ret = ::shutdown(sock_fd_, SHUT_WR);
    if (ret != 0) {
        LOG_ERROR << "shut down write on sock_fd: " << sock_fd_ << " failed";
    }
}

void Socket::set_tcp_no_delay(bool s) {
    int op = s ? 1 : 0;
    ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));
}

void Socket::set_reuse_address(bool s) {
    int op = s ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));
}

void Socket::set_reuse_port(bool s) {
    int op = s ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEPORT, &op, sizeof(op));
}

void Socket::set_keep_alive(bool s) {
    int op = s ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, &op, sizeof(op));
}