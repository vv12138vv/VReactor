#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include "log_buffer.hpp"


const size_t kSmallBufferSize = 4000;
const size_t kLargeBufferSize = 4000 * 1000;

class LogStream {
public:
    using SmallBuffer = LogBuffer<kSmallBufferSize>;

private:
    SmallBuffer buffer_;

public:
    LogStream(const LogStream& that) = delete;
    LogStream& operator=(const LogStream& that) = delete;
    void append(const char* p, int len) { buffer_.append(p, len); }
    const SmallBuffer& buffer() const { return buffer_; }
    void reset_buffer(){
        buffer_.reset();
    }

    //以下是一些<<的重载
    LogStream& operator<<(const std::string& str);

    LogStream& operator<<(const char* str);
};


#endif