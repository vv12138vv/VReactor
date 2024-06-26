#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include "timer.h"
#include <memory>
#include <set>


class TimerManager {//Todo: TimerManager实现
public:
    using TimerSet = std::set<TimePoint, std::unique_ptr<Timer>>;

private:
    TimerSet timers;
    
public:
};

#endif