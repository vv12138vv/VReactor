#include "thread.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>


std::atomic<int> Thread::count_(0);


Thread::Thread(const ThreadFunc& func, const std::string& name)
    : is_started_(false)
    , is_joined_(false)
    , func_(func)
    , name_(name) {
    Thread::count_++;
    set_name();
}


void Thread::start() {
    assert(is_started_ == false);
    thread_ = std::make_unique<std::thread>([this]() {
        is_started_ = true;
        func_();
    });
    while(!is_started_);
}


void Thread::set_name() {
    int num = Thread::count_;
    if (name_.empty()) {
        name_ = "Thread" + std::to_string(num);
    }
}

bool Thread::joinable() const {
    assert(thread_ != nullptr);
    return thread_->joinable();
}

void Thread::join() {
    is_joined_ = true;
    thread_->join();
}

bool Thread::is_started() const {
    return is_started_;
}

Thread::~Thread() {
    if (thread_ == nullptr) {
        return;
    }
    if (thread_->joinable() && !is_joined_) {
        join();
    }
}

std::thread::id Thread::get_id() const {
    return thread_->get_id();
}

void Thread::detach() {
    thread_->detach();
}