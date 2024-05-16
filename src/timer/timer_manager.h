#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include "logger.h"
#include "time_stamp.h"
#include "timer.h"
#include <atomic>
#include <map>
#include <memory>
#include <sys/timerfd.h>




class Reactor;
class Channel;

class TimerManager {   // Todo: TimerManager实现
public:
    using TimerSet = std::map<TimePoint, Timer*>;
    using TimerCallBack = std::function<void()>;

private:
    TimerSet timers_;
    Reactor& loop_;
    const int timer_fd_;
    std::atomic<bool> calling_expired_timers_;
    std::unique_ptr<Channel> channel_;

    void add_timer_in_loop(Timer* timer);
    void handle_read();
    void reset_timer_fd(int timer_fd, TimePoint expire_time);
    void reset(std::vector<std::pair<TimePoint, Timer*>>&& expired_timers,TimePoint now);
    bool insert(Timer* timer);
    void read_timer_fd(int timer_fd);
    std::vector<std::pair<TimePoint,Timer*>> get_expired_timer(TimePoint now);

public:
    TimerManager() = delete;
    explicit TimerManager(Reactor& loop);
    ~TimerManager();
    void add_timer(TimerCallBack cb, TimePoint when, Duration interval);
};

#endif