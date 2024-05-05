#ifndef LOGGER_H
#define LOGGER_H


#include "log_stream.h"
#include "timer.h"
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

// Logger前端接口
class Logger {
public:
    //日志等级
    enum Level { Trace = 0, Debug, Info, Warn, Error, Fatal };
    //输出函数类型
    using OutputFunc = std::function<void(const char* msg, size_t len)>;
    //刷新函数类型
    using FlushFunc = std::function<void()>;

private:
    static std::string simplify_file_name(const std::string& file_name);
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
        std::string base_name_;

        // Impl(Logger::Level level, int save_errno, const std::string& file_name, int line);
        Impl(const std::string& file_name, int line, int save_errno, Logger::Level level);
        void format_time();
        void finish();
    };

    // unique_ptr指向内部实现类
    std::unique_ptr<Impl> impl_;

    static OutputFunc g_output_;
    static FlushFunc g_flush_;
    static Logger::Level g_level_;

public:
    //初始化一个logger对象
    Logger(const std::string& file_name, int line, Logger::Level level = Logger::Level::Info);
    ~Logger();
    LogStream& stream() { return impl_->stream_; }
    //获取日志等级
    static Logger::Level log_level();
    //设置日志等级
    static void set_log_level(Logger::Level level);
    //设置输出
    static void set_output(OutputFunc);
    //设置刷新
    static void set_flush(FlushFunc);
    //默认输出
    static void default_output(const char* data, size_t len);
    //默认刷新
    static void default_flush();
};

extern const std::vector<std::string> level_str;

const char* get_err_msg(int save_errno);



//对外暴露的接口,实际上每次logger都是创建一个logger临时对象，然后通过RAII的方法进行输出
//__FILE__,__LINE__是一些宏

#define LOG_TRACE                                    \
    if (Logger::log_level() <= Logger::Level::Trace) \
    Logger(__FILE__, __LINE__, Logger::Level::Trace).stream()

#define LOG_DEBUG                                    \
    if (Logger::log_level() <= Logger::Level::Debug) \
    Logger(__FILE__, __LINE__, Logger::Level::Debug).stream()

#define LOG_INFO                                    \
    if (Logger::log_level() <= Logger::Level::Info) \
    Logger(__FILE__, __LINE__, Logger::Level::Info).stream()

#define LOG_WARN Logger(__FILE__, __LINE__, Logger::Level::Warn).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::Level::Error).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::Level::Fatal).stream()

#endif