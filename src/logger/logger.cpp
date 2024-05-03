#include "logger.h"
#include "timer.h"\
#include <memory>

Logger::Impl::Impl(Logger::Level level, int save_errno, const std::string& file_name, int line)
    : time_(std::chrono::time_point_cast<Duration>(BaseClock::now()))
    , stream_()
    , file_name_(file_name)
    , line_(line) {

    }

Logger::Logger(const std::string& file_name, int line, Logger::Level level)
    : impl_(std::make_unique<Impl>(level, 0, file_name, line)) {}

