#include "file_util.h"



FileUtil::FileUtil(const std::string& file_name)
    : file_(fopen(file_name.c_str(), "ae"))
    , write_offset_(0) {
    assert(file_ != nullptr);
    setbuffer(file_, buffer_, sizeof(buffer_));
}

//RTTI手法
FileUtil::~FileUtil() {
    fclose(file_);
}

//真实的写入，注意是无锁的，由调用者保证数据竞争
size_t FileUtil::write(const char* data, size_t len) {
    //无锁写入
    return fwrite_unlocked(data, 1, len, file_);
}

//添加缓存
void FileUtil::append(const char* data, size_t len) {
    //已写入的数据
    size_t written = 0;
    while (written != len) {
        //还未写入的数据
        size_t remain = len - written;
        size_t ret = FileUtil::write(data + written, remain);
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
