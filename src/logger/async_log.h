#ifndef ASYNC_LOG_H
#define ASYNC_LOG_H

#include "log_buffer.hpp"
#include "log_stream.h"
#include <vector>
#include<memory>
#include<atomic>
#include<thread>
#include<mutex>
#include<condition_variable>

class AsyncLog {
public:
    using LargeBuffer=LogBuffer<kLargeBufferSize>;
    using BufferList=std::vector<std::unique_ptr<LargeBuffer>>;
    using BufferPtr=std::unique_ptr<LargeBuffer>;


private:
    const int flush_intervals_;//前端缓冲区向后端的写入时间
    std::atomic<bool> running_;//标识线程函数是否running
    std::mutex mtx_;
    std::condition_variable cdv_;
    std::thread thread_;
    BufferPtr cur_buffer_;
    BufferPtr next_buffer_;
    BufferList buffers_;
    

    void thread_func();
public:
};


#endif