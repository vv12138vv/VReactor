#include "async_log.h"
#include <cassert>
#include <mutex>



void AsyncLog::start(){
    is_running_=true;
}

void AsyncLog::stop(){
    is_running_=false;
    cdv_.notify_one();
}


// append可能被多个前端线程调用，需要考虑线程安全。
void AsyncLog::append(const char* log, size_t len) {
    std::lock_guard<std::mutex> locker(mtx_);
    if (cur_buffer_->available() > len) {   //当前缓冲区大小足够
        cur_buffer_->append(log, len);
    } else {   //当前缓冲区大小不够
        buffers_.push_back(std::move(cur_buffer_));
        if (next_buffer_ != nullptr) {
            cur_buffer_ = std::move(next_buffer_);
        } else {
            cur_buffer_.reset(new LargeBuffer);
        }
        cur_buffer_->append(log, len);
        //当没有log消息记录时，后端线程可能阻塞等待log消息，当缓冲满时，及时唤醒后端将数据写入磁盘。大部分情况下后端就一个线程，故唤醒一个足以。
        cdv_.notify_one();
    }
}




void AsyncLog::thread_func() {
    assert(is_running_ == true);

}