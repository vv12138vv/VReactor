#ifndef TCP_SERVER_H
#define TCP_SERVER_H


#include "acceptor.h"
#include "event_loop.h"
#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"
#include "net_address.h"
#include "noncopyable.h"
#include "tcp_connection.h"
#include <memory>
#include <unordered_map>
class TcpServer : noncopyable {
public:
private:
    using ConnectionMap = std::unordered_map<std::string, std::shared_ptr<TcpConnection>>;

    EventLoop& base_loop_;
    std::atomic<size_t> started_cnt_;
    std::unique_ptr<Acceptor> accpetor_;
    std::shared_ptr<EventLoopThreadPool> thread_pool_;
    ConnectionMap connections_;
    int next_connection_idx_;
    NetAddress listen_addr_;
    std::string name_;

    //一些回调函数
    ConnectionCallBack connection_cb_;
    ReadCallBack read_cb_;
    WriteCallBack write_cb_;
    EventLoopThread::ThreadInitCallBack thread_init_cb_;

    void new_connection(int sock_fd, const NetAddress& peer_addr);
    void remove_connection(const std::shared_ptr<TcpConnection>& connection);
    void remove_connection_in_loop(const std::shared_ptr<TcpConnection>& connection);

public:
    ~TcpServer();
    TcpServer(EventLoop& base_loop, const NetAddress& listen_addr, const std::string& name);
    void set_thread_cnt(size_t cnt) { thread_pool_->set_thread_cnt(cnt); }
    void start();
};
#endif