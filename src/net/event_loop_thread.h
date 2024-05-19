#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H


#include "noncopyable.h"
#include "thread.h"
#include <condition_variable>
#include <functional>
#include <mutex>
class EventLoop;

class EventLoopThread : noncopyable {
public:
    using ThreadInitCallBack = std::function<void(EventLoop* loop)>;

private:
    void thread_func();
    EventLoop* loop_;
    bool exited_;
    Thread thread_;
    std::mutex mtx_;
    std::condition_variable cdv_;
    ThreadInitCallBack call_back_;

public:
    EventLoopThread() = delete;
    EventLoopThread(const ThreadInitCallBack& call_back, const std::string& name);
    EventLoop* start_loop();
    ~EventLoopThread();
};


#endif