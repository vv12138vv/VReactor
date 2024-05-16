#ifndef ACCEPTOR_H
#define ACCEPTOR_H



#include "channel.h"
#include "net_address.h"
#include "reactor.h"
#include "socket.h"
class Reactor;
class NetAddress;

using NewConnectionCallBack = std::function<void(int sock_fd, const NetAddress&)>;

class Acceptor {
private:
    Reactor& loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallBack new_connection_cb_;
    bool listening_;
    void handle_read();

public:
    Acceptor() = delete;
    Acceptor& operator=(const Acceptor& that) = delete;
    Acceptor(const Acceptor& that) = delete;
    Acceptor(Reactor& loop, const NetAddress& listen_addr);
    ~Acceptor();
    void set_cb(const NewConnectionCallBack& cb);
    bool is_listening() const { return listening_; }
    void listen();
};


#endif