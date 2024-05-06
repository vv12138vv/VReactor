#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "thread.h"
#include "time_stamp.h"
#include "timer_manager.h"
#include"logger.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/eventfd.h>
#include <thread>
#include <vector>

class Channel;
class Poller;

//作为poller和channel的桥梁
class Reactor {
public:
    using ChannelList = std::vector<Channel*>;
    using Func = std::function<void()>;

private:
    std::atomic<bool> looping_;             //是否在循环
    std::atomic<bool> quit_;                //是否退出
    std::atomic<bool> do_pending_func_;   //是否有需要执行的操作
    TimePoint poller_return_time_;          //发生事件的channels的返回时间
    std::unique_ptr<Poller> poller_;        // poller
    std::unique_ptr<TimerManager> timers_;
    ChannelList active_channels_;   //活跃的channels
    Channel* cur_active_channel_;
    std::mutex mtx_;
    std::vector<Func> pending_funcs_;
    const std::thread::id belong_id_;
    int wake_up_fd_;//用于唤醒线程，线程间通信的方式
    std::unique_ptr<Channel> wake_up_channel_;

    void handle_read();
    void do_pending_funcs();
public:
    Reactor();
    Reactor(const Reactor& that) = delete;
    Reactor& operator=(const Reactor& that) = delete;
    ~Reactor();
    //真正的工作循环，获得当前所有激活事件的channel，使用poller填充active_channels，然后处理每个激活的channel
    void loop();
    void quit();
    //判断当前线程是否为EventLoop的持有者线程
    bool is_in_loop_thread() const;
    void wake_up();
    void update_channel(Channel* channel);
    void remove_channel(Channel* channel);
    bool has_channel(Channel* channel);
    void run_in_loop(Func cb);
    void queue_in_loop(Func cb);
};

inline uint64_t thread_id_to_int(const std::thread::id& thread_id);

#endif