#ifndef TIMER_H
#define TIMER_H

#include "time_stamp.h"
#include <chrono>
#include <functional>



class Timer {
public:
    using CallBack = std::function<void()>;
private:
    const CallBack callback_;   //回调函数
    const bool repeatable_;     //是否是可重复的定时器
    const Duration interval_;   //重复间隔
    TimePoint expire_;          //距离下一次超时时间
public:
    Timer(CallBack&& callback, TimePoint expire, Duration interval);
    Timer(const Timer& that) = delete;
    Timer& operator=(const Timer& that) = delete;
    void call() const;
    TimePoint expire_time() const;
    bool repeatable() const;
    void restart(TimePoint now);
};


#endif