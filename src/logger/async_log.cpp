#include "async_log.h"
#include "log_file/log_file.h"


AsyncLog::AsyncLog(const std::string& base_name, off_t roll_size, int flush_interval)
    : base_name_(base_name)
    , roll_size_(roll_size)
    , flush_interval_(flush_interval)
    , thread_(std::bind(&AsyncLog::thread_write_func, this), "LogThread")
    , cur_buffer_(new LargeBuffer)
    , next_buffer_(new LargeBuffer)
    , is_running_(false) {}

//控制后端写文件线程
void AsyncLog::start() {
    is_running_ = true;
    thread_.start();
}

void AsyncLog::stop() {
    is_running_ = false;
    cdv_.notify_all();
    thread_.join();
}

AsyncLog::~AsyncLog() {
    if (is_running_) {
        stop();
    }
}

// append可能被多个前端线程调用，需要考虑线程安全。
void AsyncLog::append(const char* log, size_t len) {
    std::lock_guard<std::mutex> locker(mtx_);
    if (cur_buffer_->available() > len) {   //当前缓冲区大小足够
        cur_buffer_->append(log, len);
    } else {   //当前缓冲区大小不够
        full_buffers_.push_back(std::move(cur_buffer_));
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




void AsyncLog::thread_write_func() {
    assert(is_running_ == true);
    LogFile output(base_name_, roll_size_, flush_interval_);
    auto new_buffer1 = std::make_unique<LargeBuffer>();
    auto new_buffer2 = std::make_unique<LargeBuffer>();
    new_buffer1->set_zero();
    new_buffer2->set_zero();
    //缓冲区数组16格，用于与前端缓冲区进行交换
    BufferList buffers_for_write;   //待写缓冲队列,给前端使用，防止后端写时前端无法写数据
    buffers_for_write.reserve(16);
    while (is_running_) {
        {
            std::unique_lock<std::mutex> locker(mtx_);
            if (full_buffers_.empty()) {
                cdv_.wait_for(locker, std::chrono::seconds(flush_interval_));
            }
            full_buffers_.push_back(std::move(cur_buffer_));
            cur_buffer_ = std::move(new_buffer1);
            buffers_for_write.swap(full_buffers_);   //将前后的缓存区们交换
            if (!next_buffer_) {
                next_buffer_ = std::move(new_buffer2);
            }
        }

        //将交换来的缓冲的数据添加到LogFile
        for (const auto& buffer : buffers_for_write) {
            output.append(buffer->data(), buffer->size());
        }
        if (buffers_for_write.size() > 2) {
            buffers_for_write.resize(2);
        }
        if (!new_buffer1) {
            new_buffer1 = std::move(buffers_for_write.back());
            buffers_for_write.pop_back();
            new_buffer1->reset();
        }
        if (!new_buffer2) {
            new_buffer2 = std::move(buffers_for_write.back());
            buffers_for_write.pop_back();
            new_buffer2->reset();
        }
        buffers_for_write.clear();
        output.flush();
    }
    output.flush();
}