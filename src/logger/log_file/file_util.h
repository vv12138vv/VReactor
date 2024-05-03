#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include<stdio.h>
#include<string>
#include<assert.h>
class FileUtil {

private:
    FILE* file_;
    char buffer_[64*1024];
    off_t write_offset_;
    size_t write(const char* data,size_t len);
public:
    explicit FileUtil(const std::string& file_name);
    ~FileUtil();
    void append(const char* data,size_t len);
    void flush();
    std::streampos get_offset() const{
        return write_offset_;
    }
};


#endif