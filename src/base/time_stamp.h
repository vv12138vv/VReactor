#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include <chrono>

using BaseClock = std::chrono::system_clock;
using Us = std::chrono::microseconds;
using Duration = std::chrono::duration<size_t, std::micro>;
using TimePoint = std::chrono::time_point<BaseClock, Duration>;
const time_t kMicroSecondPerSecond=1000000;
#endif