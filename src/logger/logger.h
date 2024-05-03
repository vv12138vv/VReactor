#ifndef LOGGER_H
#define LOGGER_H


#include "log_stream.h"
#include "timer.h"
class Logger {
public:
    enum class Level { Trace, Debug, Info, Warn, Error, Fatal };

private:
    class Impl{
        TimePoint time_;
        LogStream stream_;
        Logger::Level level_;
        int line_;
        
    };
    Impl* impl_;
};

#endif