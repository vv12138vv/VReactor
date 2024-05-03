#include "file_util.h"



FileUtil::FileUtil(const std::string& file_name)
    : file_(fopen(file_name.c_str(), "ae"))
    , write_offset_(0) {
    assert(file_ != nullptr);
    setbuffer(file_, buffer_, sizeof(buffer_));
}

FileUtil::~FileUtil() {
    fclose(file_);
}


size_t FileUtil::write(const char* data, size_t len) {
    //无锁写入
    return fwrite_unlocked(data, 1, len, file_);
}


void FileUtil::append(const char* data, size_t len) {
    //已写入的数据
    size_t written = 0;
    while (written != len) {
        //还未写入的数据
        size_t remain = len - written;
        size_t ret = write(data + written, remain);
        if(ret!=remain){
            int err=ferror(file_);
            if(err){
                fprintf(stderr, "FileUtil::append failed\n");
            }
        }
        written+=ret;
    }
    write_offset_+=written;
}

void FileUtil::flush(){
    fflush(file_);
}
