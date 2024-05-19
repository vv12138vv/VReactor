#include "timer_manager.h"
#include "channel.h"
#include "reactor.h"
#include "time_stamp.h"
#include "timer.h"
#include <bits/types/struct_itimerspec.h>
#include <bits/types/time_t.h>
#include <chrono>
#include <ctime>
#include <iterator>
#include <memory>
#include <sys/timerfd.h>
#include <unistd.h>
#include <utility>

int create_time_fd() {
    int time_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (time_fd < 0) {
        LOG_ERROR << "create time_fd failed";
    }
    return time_fd;
}


TimerManager::TimerManager(EventLoop& loop)
    : loop_(loop)
    , timer_fd_(create_time_fd())
    , channel_(std::make_unique<Channel>(loop, timer_fd_))
    , timers_()
    , calling_expired_timers_(false) {
    channel_->set_read_call_back(std::bind(&TimerManager::handle_read, this));
    channel_->enable_read();
}

TimerManager::~TimerManager() {
    channel_->disable_all();
    channel_->remove_from_loop();
    ::close(timer_fd_);
}



void TimerManager::add_timer(TimerCallBack cb, TimePoint when, Duration interval) {
    loop_.run_in_loop([this, timer = new Timer(std::move(cb), when, interval)] { add_timer_in_loop(timer); });
}

void TimerManager::add_timer_in_loop(Timer* timer) {
    bool earlist_changed = insert(timer);
    if (earlist_changed) {
        reset_timer_fd(timer_fd_, timer->expire_time());
    }
}

void TimerManager::reset_timer_fd(int timer_fd, TimePoint expire_time) {
    itimerspec old_val;
    itimerspec new_val;
    memset(&old_val, 0, sizeof(old_val));
    memset(&new_val, 0, sizeof(new_val));
    Duration diff = expire_time.time_since_epoch() - std::chrono::duration_cast<Us>(BaseClock::now().time_since_epoch());
    size_t diff_us = diff.count();
    if (diff_us < 100) {
        diff_us = 100;
    }
    timespec ts;
    ts.tv_sec = static_cast<time_t>(diff_us / kMicroSecondPerSecond);
    ts.tv_nsec = static_cast<time_t>((diff_us % kMicroSecondPerSecond) * 1000);
    new_val.it_value = ts;
    if (::timerfd_settime(timer_fd, 0, &new_val, &old_val)) {
        LOG_ERROR << "reset time_fd:" << timer_fd << " failed";
    }
}

void read_timer_fd(int timer_fd) {
    size_t read_bytes = 0;
    ssize_t readn = ::read(timer_fd, &read_bytes, sizeof(read_bytes));
    if (readn != sizeof(read_bytes)) {
        LOG_ERROR << "TimerManager::read_timer_fd read_size < 0";
    }
}

std::vector<std::pair<TimePoint, Timer*>> TimerManager::get_expired_timer(TimePoint now) {
    std::vector<std::pair<TimePoint, Timer*>> expired_timers;
    auto r = timers_.lower_bound(now);
    std::move(timers_.begin(), r, std::back_inserter(expired_timers));
    timers_.erase(timers_.begin(), r);
    return expired_timers;
}

void TimerManager::handle_read() {
    TimePoint now = std::chrono::time_point_cast<Duration>(BaseClock::now());
    read_timer_fd(timer_fd_);
    auto expired_timers = std::move(get_expired_timer(now));

    calling_expired_timers_ = true;
    for (const auto& timer : expired_timers) {
        timer.second->call();
    }
    calling_expired_timers_ = false;
    reset(std::move(expired_timers), now);
}

void TimerManager::reset(std::vector<std::pair<TimePoint, Timer*>>&& expired_timers, TimePoint now) {
    for (auto& it : expired_timers) {
        if (it.second->repeatable()) {
            auto new_timer = it.second;
            new_timer->restart(std::chrono::time_point_cast<Duration>(BaseClock::now()));
            insert(new_timer);
        } else {
            delete it.second;
        }
        if (!timers_.empty()) {
            reset_timer_fd(timer_fd_, timers_.begin()->second->expire_time());
        }
    }
}

bool TimerManager::insert(Timer* timer) {
    bool earlist_changed = false;
    TimePoint when = timer->expire_time();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earlist_changed = true;
    }
    timers_.insert({when, timer});
    return earlist_changed;
}