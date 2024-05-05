#ifndef CHANNEL_H
#define CHANNEL_H
#include "time_stamp.h"
#include "timer.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <sys/epoll.h>

class Reactor;

//Channel是对一个fd的抽象，包括设置监听事件，回调函数
class Channel {
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(TimePoint)>;
    enum Event { NoneEvent = 0, ReadEvent = EPOLLIN | EPOLLPRI, WriteEvent = EPOLLOUT };

private:
    const int fd_;              //监听的fd
    Reactor& loop_;           //所属的事件循环
    int events_;                //注册的感兴趣事件
    int real_events_;           // pool返回的实际发生的事件
    int idx_;                   //注册的idx
    bool tied_;                 //是否绑定connection
    std::weak_ptr<void> tie_;   //指向对应的Connection

    //一些回调函数
    ReadEventCallBack read_call_back_;
    EventCallBack write_call_back_;
    EventCallBack close_call_back_;
    EventCallBack error_call_back_;

    //更新epoll_ctl中的事件
    void update();
    void handle_event_with_guard(TimePoint receive_time);


public:
    Channel(Reactor& loop, int fd);
    Channel()=delete;
    ~Channel();
    Channel& operator=(const Channel& that) = delete;
    Channel(const Channel& that) = delete;

    //设置一些回调函数
    void set_read_call_back(ReadEventCallBack&& call_back) { read_call_back_ = std::move(call_back); }
    void set_write_call_back(EventCallBack&& call_back) { write_call_back_ = std::move(call_back); }
    void set_close_call_back(EventCallBack&& call_back) { close_call_back_ = std::move(call_back); }
    void set_error_call_back(EventCallBack&& call_back) { error_call_back_ = std::move(call_back); }

    //一些为了封装性的函数
    int get_fd() const { return fd_; }
    int get_event() const { return events_; }
    void set_real_event(int real_event) { real_events_ = real_event; }
    int get_idx() const { return idx_; }
    void set_idx(int idx) { idx_ = idx; }

    //判断状态
    bool is_none_event() const { return events_ == Event::NoneEvent; }
    bool is_writing() const { return events_ & Event::WriteEvent; }
    bool is_reading() const { return events_ & Event::ReadEvent; }
    //设置状态
    void enable_read() {
        events_ |= Event::ReadEvent;
        update();
    }

    void disable_read() {
        events_ &= (~Event::ReadEvent);
        update();
    }
    void enable_write() {
        events_ |= Event::WriteEvent;
        update();
    }
    void disable_write() {
        events_ &= (~Event::WriteEvent);
        update();
    }

    void disable_all() {
        events_ &= Event::NoneEvent;
        update();
    }

    //绑定connection
    void tie(const std::shared_ptr<void>& obj);
    // fd收到通知后，处理事件的回调函数
    void handle_event(TimePoint receive_time);
    //从所属eventloop中删除该channel
    void remove_from_loop();
};

#endif