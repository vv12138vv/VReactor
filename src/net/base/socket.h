#ifndef SOCKET_H
#define SOCKET_H

#include "logger.h"
#include "net_address.h"
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Socket {
private:
    const int sock_fd_;

public:
    Socket() = delete;
    explicit Socket(int sock_fd)
        : sock_fd_(sock_fd) {}
    Socket(const Socket& that) = delete;
    Socket& operator=(const Socket& that) = delete;
    ~Socket();
    int get_sock_fd() const { return sock_fd_; }
    //绑定本地地址和socket
    void bind_address(const NetAddress& local_addr);
    void listen();
    int accept(NetAddress* peer_addr);
    void shut_down_write();
    //设置Nagel算法
    void set_tcp_no_delay(bool);
    //设置地址复用
    void set_reuse_address(bool);
    //设置端口复用
    void set_reuse_port(bool);
    //设置长连接
    void set_keep_alive(bool);
};

#endif