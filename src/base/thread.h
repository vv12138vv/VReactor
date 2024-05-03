#ifndef THREAD_H
#define THREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class Thread {
public:
    using ThreadFunc = std::function<void()>;

private:
    std::mutex mtx_;
    std::unique_ptr<std::thread> thread_;
    bool is_started_;
    bool is_joined_;
    ThreadFunc func_;
    std::string name_;
    static std::atomic<int> count_;   //创建的线程数

    //初始化线程名
    void set_name();

public:
    explicit Thread(const ThreadFunc& func, const std::string& name);
    ~Thread();
    void start();
    void join();
    bool joinable() const;
    bool is_started() const;
    std::thread::id get_id() const;
    void detach();
};


#endif