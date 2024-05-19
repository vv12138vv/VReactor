#include "event_loop_thread_pool.h"
#include "event_loop.h"
#include <string>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const std::string& name)
    : base_loop_(base_loop)
    , name_(name)
    , is_started_(false)
    , thread_cnt_(0)
    , next_idx_(0) {}


void EventLoopThreadPool::start(const EventLoopThread::ThreadInitCallBack& call_back) {
    is_started_ = true;
    for (int i = 0; i < thread_cnt_; i += 1) {
        std::string thread_name = name_ + std::to_string(i);
        threads_.emplace_back(call_back, thread_name);
        loops_.push_back(threads_.back()->start_loop());
    }
    if (thread_cnt_ == 0 && call_back != nullptr) {
        call_back(base_loop_);
    }
}

EventLoop* EventLoopThreadPool::next_loop() {
    EventLoop* loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[next_idx_];
        next_idx_ += 1;
        if (next_idx_ > loops_.size() - 1) {
            next_idx_ = 0;
        }
    }
    return loop;
}


std::vector<EventLoop*> EventLoopThreadPool::get_all_loops() {
    if (loops_.empty()) {
        return std::vector<EventLoop*>{base_loop_};
    }
    return loops_;
}