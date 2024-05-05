#ifndef LOGGER_H
#define LOGGER_H


#include "log_stream.h"
#include "timer.h"
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>




// Logger前端接口
class Logger {
public:
    //日志等级
    enum Level { Trace, Debug, Info, Warn, Error, Fatal };
    //输出函数类型
    using OutputFunc = std::function<void(const char* msg, size_t len)>;
    //刷新函数类型
    using FlushFunc = std::function<void()>;

private:
    //用于取文件名的最后一段,内部类
    class FileName {
    public:
        std::string data_;
        size_t len_;

        explicit FileName(const std::string& filename) {
            data_ = filename;
            size_t slashPos = data_.find_last_of('/');
            if (slashPos != std::string::npos) {
                data_ = data_.substr(slashPos + 1);
            }
            len_ = data_.length();
        }
    };
    //内部实现类
    class Impl {
    public:
        //时间戳
        TimePoint time_;
        //日志流
        LogStream stream_;
        //日志等级
        Logger::Level level_;
        //日志输出所在行
        int line_;
        //所在文件的最后一段文件名
        FileName base_name_;

        Impl(Logger::Level level, int save_errno, const std::string& file_name, int line);
        void format_time();
        void finish();
    };
    // unique_ptr指向内部实现类
    std::unique_ptr<Impl> impl_;

    static OutputFunc g_output;
    static FlushFunc g_flush;
    static void default_output(const char* data, size_t len);
    static void default_flush();

public:
    //初始化一个logger对象
    Logger(const std::string& file_name, int line, Logger::Level level = Logger::Level::Info);
    ~Logger();
    LogStream& stream() { return impl_->stream_; }
    static Logger::Level log_level();
    static void set_log_level(Logger::Level level);
    static void set_output(OutputFunc);
    static void set_flush(FlushFunc);
};
extern Logger::Level g_level;

extern const std::unordered_map<Logger::Level, std::string> level_str;
// const char* get_err_msg(int save_errno);
inline Logger::Level Logger::log_level() {
    return g_level;
}



//对外暴露的接口
#define LOG_DEBUG if (Logger::log_level() <= Logger::Level::Debug) \
    Logger(__FILE__, __LINE__, Logger::Level::Debug).stream()

#define LOG_INFO if (Logger::log_level() <= Logger::Level::Info) \
    Logger(__FILE__, __LINE__, Logger::Level::Info).stream()

#define LOG_WARN Logger(__FILE__, __LINE__, Logger::Level::Warn).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::Level::Error).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::Level::Fatal).stream()

#endif