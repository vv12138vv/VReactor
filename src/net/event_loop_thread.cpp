#include "event_loop_thread.h"
#include "event_loop.h"
#include "timer_manager.h"
#include <cstddef>
#include <mutex>

EventLoopThread::EventLoopThread(const ThreadInitCallBack& call_back, const std::string& name)
    : loop_(nullptr)
    , exited_(false)
    , mtx_()
    , cdv_()
    , call_back_(call_back)
    , thread_(std::bind(&EventLoopThread::thread_func, this), name) {}



EventLoop* EventLoopThread::start_loop() {
    thread_.start();
    EventLoop* loop=nullptr;
    {
        std::unique_lock<std::mutex> locker(mtx_);
        while(loop_==nullptr){
            cdv_.wait(locker);
        }
    }
    loop_=loop;
    return loop;
}


void EventLoopThread::thread_func() {
    EventLoop loop;
    if (call_back_ != nullptr) {
        call_back_(&loop);
    }
    {
        std::unique_lock<std::mutex> locker(mtx_);
        loop_ = &loop;
        cdv_.notify_one();
    }
    loop.loop();
    std::unique_lock<std::mutex> locker(mtx_);
    loop_ = nullptr;
}

EventLoopThread::~EventLoopThread(){
    exited_=true;
    if(loop_!=nullptr){
        loop_->quit();
        thread_.join();
    }
}