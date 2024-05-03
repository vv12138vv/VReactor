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
    const int flush_intervals_;      //前端缓冲区向后端的写入时间
    std::atomic<bool> is_running_;   //标识线程函数是否running
    std::mutex mtx_;                 //
    std::condition_variable cdv_;
    Thread thread_;           //后端线程
    
    BufferPtr cur_buffer_;    //当前缓冲区
    BufferPtr next_buffer_;   //空闲缓冲区
    BufferList buffers_;      //已满队列缓冲


    void thread_func();

public:
    ~AsyncLog();
    void append(const char* log, size_t len);
    void start();
    void stop();
};


#endif