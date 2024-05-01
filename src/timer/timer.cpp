#include "timer.h"

Timer::Timer(Timer::CallBack&& callback, Timer::TimePoint expire, Timer::Duration interval = Us(0))
    : callback_(std::move(callback))
    , expire_(expire)
    , interval_(interval)
    , repeatable_(interval > Us(0)) {}

void Timer::call() const {
    callback_();
}

Timer::TimePoint Timer::expire_time() const {
    return expire_;
}

bool Timer::repeatable() const {
    return repeatable_;
}

void Timer::restart(Timer::TimePoint now) {
    if (!repeatable_) {
        expire_ = TimePoint(Us(0));
        return;
    }
    expire_ = now + interval_;
}
