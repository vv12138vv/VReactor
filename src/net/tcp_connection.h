#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "buffer.h"
#include "net_address.h"
#include "reactor.h"
#include <atomic>
#include <memory>
#include <string>


class Channel;
class Reactor;
class Socket;

class TcpConnection {
public:
private:
    enum State { Connecting, Connected, Disconnected, Disconnecting };
    Reactor& loop_;
    const std::string name_;
    std::atomic<int> state_;   //连接状态
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const NetAddress local_addr_;   //本地地址
    const NetAddress peer_addr_;    //对端地址

    Buffer input_buffer_;    //接收缓冲区
    Buffer output_buffer_;   //发送缓冲区

    size_t limit_;

public:
    TcpConnection(Reactor& loop, const std::string& name, int sock_fd, const NetAddress& local_addr, const NetAddress& peer_addr);
};


#endif