#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "log_file/file_util.h"
#include <bits/types/time_t.h>
#include <memory>
#include <mutex>
#include <string>

class LogFile {
private:
    const std::string base_name_;   //基础文件名
    const int flush_interval_;      //刷新间隔
    const off_t roll_size_;         //日志滚动限值
    const int write_limit_;         //写数据次数限制
    size_t write_count_;            //写数据次数计数
    time_t last_roll_;              //上次roll日志时间(s)
    time_t last_flush_;             //上次flush日志时间
    time_t start_of_period_;        //本次写log周期起始时间
    std::unique_ptr<FileUtil> file_;
    std::unique_ptr<std::mutex> mtx_;

    static const int kRollPerSec = 60 * 60 * 24;

    static std::string next_log_file_name(const std::string& base_name, time_t* now);

    void append_unlocked(const char* log,size_t len);
public:
    LogFile(const std::string& base_name, off_t roll_size, bool thread_safe = true, int flush_interval = 3, int write_limit = 1024);
    void append(const char* log, size_t len);
    void flush();
    bool roll_file();
};

#endif