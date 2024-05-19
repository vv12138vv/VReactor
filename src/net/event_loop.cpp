#include "event_loop.h"
#include "channel.h"
#include "epoller.h"
#include "logger.h"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

const int kPoolTimeOutMs = 10000;


thread_local EventLoop* t_loop_in_this_thread = nullptr;

// eventfd是Linux特有的，用于事件通知的机制，evnetfd用于唤醒loop
int create_event_fd() {
    int ev_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (ev_fd == -1) {
        LOG_FATAL << "create_event_fd error:" << errno;
    }
    return ev_fd;
}



EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , do_pending_func_(false)
    , belong_id_(std::this_thread::get_id())
    , poller_(std::make_unique<Epoller>(*this))
    , timers_()
    , active_channels_()
    , cur_active_channel_(nullptr)
    , pending_funcs_()
    , wake_up_fd_(create_event_fd())
    , wake_up_channel_(std::make_unique<Channel>(*this, wake_up_fd_)) {
    LOG_DEBUG << "EventLoop is created " << this << " in thread " << std::to_string(thread_id_to_int(belong_id_));
    LOG_DEBUG << "EventLopp created wake_up_fd " << std::to_string(wake_up_channel_->get_fd());
    if (t_loop_in_this_thread != nullptr) {
        LOG_FATAL << "Another EventLoop exists in the thread " << std::to_string(thread_id_to_int(belong_id_));
    } else {
        t_loop_in_this_thread = this;
    }
    wake_up_channel_->set_read_call_back(std::bind(&EventLoop::handle_read, this));
    wake_up_channel_->enable_read();
}

EventLoop::~EventLoop() {
    wake_up_channel_->disable_all();
    wake_up_channel_->remove_from_loop();
    close(wake_up_fd_);
    t_loop_in_this_thread = nullptr;
}

uint64_t thread_id_to_int(const std::thread::id& thread_id) {
    return *((std::thread::native_handle_type*)(&thread_id));
}

void EventLoop::loop() {
    assert(looping_ == false);
    assert(is_in_loop_thread());
    looping_ = true;
    quit_ = false;
    LOG_INFO << "EventLoop " << this << " start looping";
    while (!quit_) {
        active_channels_.clear();
        poller_return_time_ = poller_->poll(kPoolTimeOutMs, active_channels_);
        for (Channel* channel : active_channels_) {
            channel->handle_event(poller_return_time_);
        }
        do_pending_funcs();
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (is_in_loop_thread()) {
        wake_up();
    }
}

bool EventLoop::is_in_loop_thread() const {
    return belong_id_ == std::this_thread::get_id();
}

void EventLoop::wake_up() {
    uint64_t one = 1;
    ssize_t n = write(wake_up_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wake_up writes " << static_cast<int>(n) << " bytes instead of 8";
    }
}

void EventLoop::handle_read() {
    uint64_t one = 1;
    ssize_t n = read(wake_up_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::update_channel(Channel* channel) {
    poller_->update_channel(channel);
}

void EventLoop::remove_channel(Channel* channel) {
    poller_->remove_channel(channel);
}

bool EventLoop::has_channel(Channel* channel) {
    return poller_->has_channel(channel);
}

void EventLoop::do_pending_funcs() {
    std::vector<Func> funcs;
    do_pending_func_ = true;
    {
        std::unique_lock<std::mutex> locker(mtx_);
        funcs.swap(pending_funcs_);
    }
    for (const Func& func : funcs) {
        func();
    }
    do_pending_func_ = false;
}

void EventLoop::run_in_loop(Func cb) {
    if (is_in_loop_thread()) {
        cb();
    } else {
        queue_in_loop(cb);
    }
}

void EventLoop::queue_in_loop(Func cb) {
    {
        std::unique_lock<std::mutex> locker(mtx_);
        pending_funcs_.push_back(std::move(cb));
    }
    if (!is_in_loop_thread() || do_pending_func_) {
        wake_up();
    }
}