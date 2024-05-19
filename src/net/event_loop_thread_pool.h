#ifndef EVENT_LOOP_THREAD_POOL_H
#define EVENT_LOOP_THREAD_POOL_H

#include "event_loop_thread.h"
#include "noncopyable.h"
#include <vector>

class EventLoop;
class EventLoopThread;


class EventLoopThreadPool : noncopyable {
public:
private:
    EventLoop* base_loop_;
    std::string name_;
    bool is_started_;
    size_t thread_cnt_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    size_t next_idx_;

public:
    EventLoopThreadPool(EventLoop* base_loop, const std::string& name);
    ~EventLoopThreadPool() = default;

    void set_thread_cnt(size_t cnt);
    void start(const EventLoopThread::ThreadInitCallBack& call_back);
    EventLoop* next_loop();
    bool started() const { return is_started_; };
    const std::string name() const { return name_; }
    std::vector<EventLoop*> get_all_loops();
};


#endif