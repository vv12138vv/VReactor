#include "channel.h"
#include "event_loop.h"
#include "time_stamp.h"


Channel::Channel(Reactor& loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(Event::NoneEvent)
    , real_events_(Event::NoneEvent)
    , idx_(-1)
    , tied_(false) {}


Channel::~Channel() {}

//与connection绑定
void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}


void Channel::handle_event(TimePoint receive_time) {
    if (tied_) {
        auto guard = tie_.lock();
        if (guard) {
            handle_event_with_guard(receive_time);
        }
    } else {
        handle_event_with_guard(receive_time);
    }
}

void Channel::handle_event_with_guard(TimePoint receive_time) {
    //对端关闭连接事件
    if ((real_events_ & EPOLLHUP) && !(real_events_ & EPOLLIN)) {
        if (close_call_back_) {
            close_call_back_();
        }
    }
    //错误事件
    if (real_events_ & EPOLLERR) {
        // Todo log
        if (error_call_back_) {
            error_call_back_();
        }
    }
    //读事件
    if (real_events_ & (EPOLLIN | EPOLLPRI)) {
        // Todo log
        if (read_call_back_) {
            read_call_back_(receive_time);
        }
    }
    //写事件
    if (real_events_ & EPOLLOUT) {
        if (write_call_back_) {
            write_call_back_();
        }
    }
}

void Channel::remove_from_loop() {
    loop_.remove_channel(this);
}