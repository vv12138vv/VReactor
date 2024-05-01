#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <functional>

class Timer {
public:
    using BaseClock = std::chrono::steady_clock;
    using Us = std::chrono::microseconds;
    using Duration = std::chrono::duration<size_t, std::micro>;
    using TimePoint = std::chrono::time_point<BaseClock, Duration>;
    using CallBack = std::function<void()>;

private:
    const CallBack callback_;   //回调函数
    const bool repeatable_;     //是否是可重复的定时器
    const Duration interval_;   //重复间隔
    TimePoint expire_;          //下一次超时时间
public:
    Timer(CallBack&& callback, TimePoint expire, Duration interval);
    void call() const;
    TimePoint expire_time() const;
    bool repeatable() const;
    void restart(TimePoint now);
};


#endif