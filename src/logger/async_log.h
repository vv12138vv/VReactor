#ifndef ASYNC_LOG_H
#define ASYNC_LOG_H

#include "log_buffer.hpp"
#include "log_stream.h"
#include "thread.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

//异步日志后端
class AsyncLog {
public:
    using LargeBuffer = LogBuffer<kLargeBufferSize>;
    using BufferList = std::vector<std::unique_ptr<LargeBuffer>>;
    using BufferPtr = std::unique_ptr<LargeBuffer>;

private:
    const std::string base_name_;
    const off_t roll_size_;
    const int flush_interval_;      //刷新间隔
    std::atomic<bool> is_running_;   //标识线程函数是否running
    std::mutex mtx_;
    std::condition_variable cdv_;
    Thread thread_;   //后端线程

    BufferPtr cur_buffer_;    //当前缓冲区
    BufferPtr next_buffer_;   //空闲缓冲区
    BufferList full_buffers_;      //已满的缓冲区

    //后台线程执行的函数
    void thread_write_func();

public:
    AsyncLog(const std::string& base_name,off_t roll_size,int flush_interval=3);
    ~AsyncLog();
    //提供给前端的接口，Log前端写入缓冲区时需要加锁
    void append(const char* log, size_t len);
    void start();
    void stop();
};


#endif