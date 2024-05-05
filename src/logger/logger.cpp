#include "logger.h"
#include "log_stream.h"
#include "timer.h"
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <process.h>
#include <string>

namespace ThreadInfo {
thread_local char t_errnobuf[512];
thread_local char t_time[64];
thread_local time_t t_last_second;
};   // namespace ThreadInfo


const std::unordered_map<Logger::Level, std::string> level_str{
    {Logger::Level::Trace, "Trace"},
    {Logger::Level::Debug, "Debug"},
    {Logger::Level::Info, "Info"},
    {Logger::Level::Warn, "Warn"},
    {Logger::Level::Error, "Error"},
    {Logger::Level::Fatal, "Fatal"},
};

Logger::Impl::Impl(Logger::Level level, int save_errno, const std::string& file_name, int line)
    : time_(std::chrono::time_point_cast<Duration>(BaseClock::now()))
    , stream_()
    , base_name_(file_name)
    , line_(line) {}


void Logger::Impl::format_time() {
    TimePoint now = std::chrono::time_point_cast<Duration>(BaseClock::now());
    time_t seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    int us_per_s = 1000 * 1000;   // 1s=1000000us
    int micro_seconds = now.time_since_epoch().count() % us_per_s;
    tm* tm_time = localtime(&seconds);   //时间解析
    const char* time_format = "%4d/%02d/%02d %02d:%02d:%02d";
    std::snprintf(ThreadInfo::t_time,
                  sizeof(ThreadInfo::t_time),
                  time_format,
                  tm_time->tm_year + 1900,
                  tm_time->tm_mon + 1,
                  tm_time->tm_mday,
                  tm_time->tm_hour,
                  tm_time->tm_min,
                  tm_time->tm_sec);
    ThreadInfo::t_last_second = seconds;
    char buf[32] = {0};
    std::snprintf(buf, sizeof(buf), "%06d", micro_seconds);
    stream_ << LogTemplate(ThreadInfo::t_time, strlen(time_format)) << LogTemplate(buf, strlen(buf));
}

void Logger::Impl::finish() {
    stream_ << "-" << LogTemplate(base_name_.data_.data(), base_name_.len_) << ":" << std::to_string(line_) << "\n";
}


Logger::Logger(const std::string& file_name, int line, Logger::Level level)
    : impl_(std::make_unique<Impl>(level, 0, file_name, line)) {}


Logger::~Logger() {
    impl_->finish();
    const LogStream::SmallBuffer& buf(stream().buffer());
    g_output(buf.data(),buf.size());
    if(impl_->level_==Level::Fatal){
        g_flush();
        abort();
    }
}

// const char* get_err_msg(int save_errno) {
//     return strerror_r(save_errno, ThreadInfo::t_errnobuf, sizeof(ThreadInfo::t_errnobuf));
// }

void Logger::default_output(const char* data, size_t len) {
    fwrite(data, len, sizeof(char), stdout);
}

void Logger::default_flush() {
    fflush(stdout);
}

void Logger::set_output(OutputFunc output_func) {
    g_output = output_func;
}
void Logger::set_flush(FlushFunc flush_func) {
    g_flush = flush_func;
}

//一些默认设置，默认输出到std::out
Logger::OutputFunc Logger::g_output = default_output;
Logger::FlushFunc Logger::g_flush = default_flush;
Logger::Level g_level = Logger::Level::Info;

void Logger::set_log_level(Logger::Level level) {
    g_level = level;
}