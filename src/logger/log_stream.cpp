#include"log_stream.h"
#include <cstring>

LogStream& LogStream::operator<<(const std::string &str){
    buffer_.append(str.c_str(), str.size());
    return *this;
}


LogStream& LogStream::operator<<(const char* str){
    if(str==nullptr){
        buffer_.append("(null)", 6);
    }else{
        buffer_.append(str, strlen(str));
    }
    return *this;
}
