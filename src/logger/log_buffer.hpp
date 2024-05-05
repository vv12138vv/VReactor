#ifndef LOG_BUFFER_HPP
#define LOG_BUFFER_HPP

#include <cstring>
#include <iostream>
#include <strings.h>



template<size_t SIZE>
class LogBuffer {
private:
    char data_[SIZE];
    char* cur_;
    const char* const end() const { return data_ + sizeof(data_); }

public:
    LogBuffer()
        : cur_(data_) {}

    LogBuffer(const LogBuffer& that) = delete;
    LogBuffer& operator=(const LogBuffer& that) = delete;

    void append(const char* p, size_t len) {
        if (available() > len) {
            memmove(cur_, p, len);
            cur_ += len;
        }
    }
    void reset() { cur_ = data_; }
    const char* data() const { return data_; }
    //已存放的数据
    size_t size() const { return static_cast<size_t>(cur_ - data_); }
    char* cur() { return cur_; }
    //缓冲区置0
    void bzero() { bzero(data_, sizeof(data_)); }
    //剩余空间
    size_t available() const { return static_cast<size_t>(end() - cur_); }
    size_t capacity() const { return sizeof(data_); }
};

#endif