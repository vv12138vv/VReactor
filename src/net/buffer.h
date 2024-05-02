#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include <sys/types.h>
#include<sys/uio.h>
#include<unistd.h>


class Buffer {
public:
private:
    std::vector<char> buffer_;
    size_t read_start_idx_;//针对于本buffer，则向外写时调整read_idx
    size_t write_start_idx_;//向buffer写时调整write

    //给buffer扩容size
    void extend(size_t size);

public:
    static const size_t kHead;
    static const size_t kBufferSize;
    explicit Buffer(size_t buffer_size = kBufferSize);
    size_t readable_size() const { return write_start_idx_ - read_start_idx_; }
    size_t writable_size() const { return buffer_.size() - write_start_idx_; }
    size_t head_size() const { return read_start_idx_; }
    //readable peek
    const char* peek() const { return &(*buffer_.begin()) + read_start_idx_; }
    //仅用于调整buffer内部的下标，不读取数据
    void retrieve(size_t len);
    void retrieve_all();

    std::string retrieve_str(size_t len);
    std::string retrieve_all_str();
    void ensure_writable_space(size_t len);
    //将临时buffer上的数据加入到buffer_中
    void append(const char* data,size_t len);
    //从fd中读取数据到buffer
    ssize_t read_fd(int fd, int* save_errno);
    //从buffer中写入数据到fd
    ssize_t write_fd(int fd, int* save_errno);
};

#endif