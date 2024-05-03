#ifndef LOGGER_H
#define LOGGER_H


#include "log_stream.h"
#include "timer.h"
#include <string>
#include<memory>




class Logger {
public:
    enum class Level { Trace, Debug, Info, Warn, Error, Fatal };
    Logger(const std::string& file_name,int line,Logger::Level level=Logger::Level::Info);
private:
    class Impl{
    public:
        TimePoint time_;
        LogStream stream_;
        Logger::Level level_;
        int line_;
        std::string file_name_;
        //SourceFile?

        Impl(Logger::Level level,int save_errno,const std::string& file_name,int line);
    };
    std::unique_ptr<Impl> impl_;
};

#endif