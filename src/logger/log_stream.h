#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include "log_buffer.hpp"
#include <cassert>

const size_t kSmallBufferSize = 4000;
const size_t kLargeBufferSize = 4000 * 1000;

//日志输出
class LogTemplate {
public:
    const char* data_;
    size_t len_;
    LogTemplate()
        : data_(nullptr)
        , len_(0) {}
    LogTemplate(const char* data, size_t len)
        : data_(data)
        , len_(len) {}
    LogTemplate(const LogTemplate&) = delete;
    LogTemplate& operator=(const LogTemplate&) = delete;
};

//日志流，主要是为了通过<<操作进行输出
class LogStream {
public:
    using StreamBuffer = LogBuffer<kSmallBufferSize>;

private:
    StreamBuffer buffer_;
    //添加到流
    void append(const char* p, int len) {
        assert(len < buffer_.available());
        buffer_.append(p, len);
    }

public:
    LogStream() = default;
    LogStream(const LogStream& that) = delete;
    LogStream& operator=(const LogStream& that) = delete;

    const StreamBuffer& buffer() const { return buffer_; }
    void reset_buffer() { buffer_.reset(); }
    //以下是一些<<的重载
    LogStream& operator<<(const std::string& str);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const LogTemplate& log_template);
    LogStream& operator<<(char ch);
    LogStream& operator<<(const void* data);
};




#endif