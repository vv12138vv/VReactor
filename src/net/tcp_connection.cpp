#include "tcp_connection.h"
#include "channel.h"
#include "net_address.h"
#include "reactor.h"
#include "socket.h"
#include <memory>

TcpConnection::TcpConnection(Reactor& loop, const std::string& name, int sock_fd, const NetAddress& local_addr, const NetAddress& peer_addr)
    : loop_(loop)
    , name_(name)
    , state_(State::Connecting)
    , reading_(true)
    , socket_(std::make_unique<Socket>(sock_fd))
    , channel_(std::make_unique<Channel>(loop, sock_fd))
    , local_addr_(local_addr)
    , peer_addr_(peer_addr)
    , limit_(64 * 1024 * 1024) {}
