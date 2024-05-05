#include "buffer.h"
#include <cerrno>


const size_t Buffer::kHead = 8;
const size_t Buffer::kBufferSize = 1024;


Buffer::Buffer(size_t buffer_size)
    : buffer_(kHead + buffer_size)
    , read_start_idx_(kHead)
    , write_start_idx_(kHead) {
    assert(readable_size() == 0);
    assert(writable_size() == buffer_size);
    assert(head_size() == kHead);
}


void Buffer::retrieve(size_t len) {
    assert(len <= readable_size());
    if (len < readable_size()) {
        read_start_idx_ += len;
    } else {
        retrieve_all();
    }
}

void Buffer::retrieve_all() {
    read_start_idx_ = kHead;
    write_start_idx_ = kHead;
}

// buffer_的扩容相关操作
void Buffer::extend(size_t size) {
    if (writable_size() + head_size() < size + kHead) {   //整个buffer不够用，则需扩容
        buffer_.resize(write_start_idx_ + size);
    } else {   // buffer够用，进行移动
        size_t readable = readable_size();
        std::copy(buffer_.begin() + read_start_idx_, buffer_.begin() + write_start_idx_, buffer_.begin() + kHead);
        read_start_idx_ = kHead;
        write_start_idx_ = read_start_idx_ + readable;
    }
}

void Buffer::ensure_writable_space(size_t size){
    if(writable_size()<size){
        extend(size);
    }
}

void Buffer::append(const char *data, size_t size){
    ensure_writable_space(size);
    std::copy(data,data+size,buffer_.begin()+write_start_idx_);
    write_start_idx_+=size;
}

/*
 * 从fd上读取数据时，并不知道要读的tcp数据的最终大小
 * 为此，采用先尽量读入buffer_，若buffer_的空间不够，则读到在栈上的临时空间中，
 *然后再将栈里的数据添加到buffer_中，可以减少系统调用的开销，又不影响数据的接收。
 *
 */
ssize_t Buffer::read_fd(int fd, int* save_errno) {
    //栈上的临时缓冲区，暂存buffer_放不下的数据：65536/1024=64kB
    char temp_buff[65536];
    //为了搭配readv和writev进行高效I/O，允许同时写入读取或写入多个非连续缓冲区
    size_t writable = writable_size();
    iovec vec[2];
    //第一块缓冲区，指向buffer_
    vec[0].iov_base = &(*buffer_.begin()) + write_start_idx_;
    vec[0].iov_len = writable;
    //第二块缓冲区，指向栈上的临时空间
    vec[1].iov_base = temp_buff;
    vec[1].iov_len = sizeof(temp_buff);
    //readv可以将fd中的数据按顺序填充到多个不连续的缓冲区中。返回值为读入的字节数
    ssize_t ret = readv(fd, vec, 2);
    if(ret==-1){
        *save_errno=errno;
    }else if(ret<=writable){//buffer的可写缓冲区已足够存储读到的数据
        write_start_idx_+=ret;
    }else{//有一部分数据读取到了temp_buff中,说明此时buffer_已读满
        write_start_idx_=buffer_.size();
        append(temp_buff, ret-writable);
    }
    return ret;
}



ssize_t Buffer::write_fd(int fd, int *save_errno){
    ssize_t ret=write(fd,peek(),readable_size());
    if(ret==-1){
        *save_errno=errno;
    }
    return ret;
}

std::string Buffer::retrieve_str(size_t len){
    assert(len<=readable_size());
    std::string res(peek(),len);
    retrieve(len);
    return res;
}
std::string Buffer::retrieve_all_str(){
    return retrieve_str(readable_size());
}