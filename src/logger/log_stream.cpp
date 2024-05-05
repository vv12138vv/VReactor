#include "log_stream.h"
#include <cstring>

LogStream& LogStream::operator<<(const std::string& str) {
    buffer_.append(str.c_str(), str.size());
    return *this;
}


LogStream& LogStream::operator<<(const char* str) {
    if (str == nullptr) {
        buffer_.append("(null)", 6);
    } else {
        buffer_.append(str, strlen(str));
    }
    return *this;
}

LogStream& LogStream::operator<<(const LogTemplate& log_template) {
    buffer_.append(log_template.data_, log_template.len_);
    return *this;
}

LogStream& LogStream::operator<<(char ch) {
    buffer_.append(&ch, 1);
    return *this;
}

LogStream& LogStream::operator<<(const void* data) {
    *this << static_cast<const char*>(data);
    return *this;
}