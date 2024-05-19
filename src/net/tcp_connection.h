#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "buffer.h"
#include "net_address.h"
#include "event_loop.h"
#include "time_stamp.h"
#include <atomic>
#include <cstdio>
#include <functional>
#include <memory>
#include <netinet/tcp.h>
#include <string>
class Channel;
class EventLoop;
class Socket;
class TcpConnection;

using ConnectionCallBack = std::function<void(const std::shared_ptr<TcpConnection>&)>;
using CloseCallBack = std::function<void(const std::shared_ptr<TcpConnection>&)>;
using WriteCallBack = std::function<void(const std::shared_ptr<TcpConnection>&)>;
using ReadCallBack = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*, TimePoint)>;
using OverLimitCallBack = std::function<void(const std::shared_ptr<TcpConnection>&, size_t)>;


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
private:
    enum State { Connecting, Connected, Disconnected, Disconnecting };
    EventLoop& loop_;
    const std::string name_;
    std::atomic<int> state_;   //连接状态
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const NetAddress local_addr_;   //本地地址
    const NetAddress peer_addr_;    //对端地址

    Buffer input_buffer_;    //接收缓冲区
    Buffer output_buffer_;   //发送缓冲区

    size_t send_limit_;   // fa

    ConnectionCallBack connection_cb_;
    CloseCallBack close_cb_;
    WriteCallBack write_cb_;
    ReadCallBack read_cb_;
    OverLimitCallBack over_limit_cb_;

    void set_state(State state) { state_ = state; }

    void handle_read(TimePoint receive_time);
    void handle_write();
    void handle_close();
    void handle_error();

    void send_in_loop(const void* data, size_t len);
    void send_in_loop(const std::string& data);
    void shutdown_in_loop();

public:
    TcpConnection() = delete;
    TcpConnection(const TcpConnection& that) = delete;
    TcpConnection& operator=(const TcpConnection& that) = delete;
    TcpConnection(EventLoop& loop, const std::string& name, int sock_fd, const NetAddress& local_addr, const NetAddress& peer_addr);
    ~TcpConnection();


    bool is_connected() const { return state_ == State::Connected; }
    std::string get_name() const { return name_; }
    NetAddress get_local_addr() const { return local_addr_; }
    NetAddress get_peer_addr() const { return peer_addr_; }

    void send(const std::string& data);
    void send(Buffer* data);
    void shutdown();

    void set_connection_callback(const ConnectionCallBack& cb) { connection_cb_ = cb; }
    void set_close_callback(const CloseCallBack& cb) { close_cb_ = cb; }
    void set_write_callback(const WriteCallBack& cb) { write_cb_ = cb; }
    void set_msg_callback(const ReadCallBack& cb) { read_cb_ = cb; }
    void set_over_limit_callback(const OverLimitCallBack& cb) { over_limit_cb_ = cb; }

    void connection_established();
    void connection_destroyed();
};



#endif