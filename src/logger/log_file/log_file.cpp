#include "log_file.h"
#include "log_file/file_util.h"
#include <cassert>
#include <ctime>
#include <memory>
#include <mutex>


LogFile::LogFile(const std::string& base_name, off_t roll_size, bool thread_safe, int flush_interval, int write_limit)
    : base_name_(base_name)
    , roll_size_(roll_size)
    , flush_interval_(flush_interval)
    , write_limit_(write_limit)
    , write_count_(0)
    , start_of_period_(0)
    , last_roll_(0)
    , last_flush_(0)
    , mtx_(thread_safe ? std::make_unique<std::mutex>() : nullptr) {
    assert(base_name.find('/') == std::string::npos);
    roll_file();
}


//当日志文件接近滚动限值时，需要换一个新文件写数据，便于归档查看，调用，仅负责滚动，判断滚动在调用着
bool LogFile::roll_file() {
    time_t now = 0;
    auto file_name = next_log_file_name(base_name_, &now);
    time_t start = now / kRollPerSec * kRollPerSec;//计算现在所处的滚动周期数
    if (now > last_roll_) {
        last_roll_ = now;
        last_flush_ = now;
        start_of_period_ = start;
        // Unique指针reset后会调用原对象的析构函数
        file_.reset(new FileUtil(file_name));
        return true;
    }
    return false;
}

//根据基础名，根据时间获得一个全新的唯一的log名：basename+now+.log
std::string LogFile::next_log_file_name(const std::string& base_name, time_t* now) {
    std::string file_name;
    file_name.reserve(base_name.size() + 64);
    file_name = base_name;
    tm tmbuf;
    *now = time(nullptr);
    // x线程安全，将时间戳转为本地时间
    localtime_r(now, &tmbuf);
    char time_buff[32];
    strftime(time_buff, sizeof(time_buff), ".%Y%m%d-%H%M%S", &tmbuf);
    file_name += time_buff;
    file_name += ".log";

    return file_name;
}

void LogFile::flush() {
    file_->flush();
}

void LogFile::append(const char *log, size_t len){
    if(mtx_!=nullptr){
        std::lock_guard<std::mutex> locker(*mtx_);
        append_unlocked(log, len);
    }else{
        append_unlocked(log, len);
    }
}

void LogFile::append_unlocked(const char* log,size_t len){
    file_->append(log, len);
    if(file_->get_offset()>roll_size_){//判断是否需要滚动
        roll_file();
    }else{
        write_count_+=1;
        if(write_count_>=write_limit_){
            write_count_=0;
            time_t now=time(nullptr);
            time_t period=now/kRollPerSec*kRollPerSec;
            if(period!=start_of_period_){
                roll_file();
            }else if(now-last_flush_>flush_interval_){
                last_flush_=now;
                file_->flush();
            }
        }
    }
}