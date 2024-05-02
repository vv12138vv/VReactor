#ifndef EPOLLER_H
#define EPOLLER_H

#include "channel.h"
#include "poller.h"
#include "sys/epoll.h"
#include <memory>
#include <unistd.h>

class Epoller : public Poller {
public:
    using EventList = std::vector<epoll_event>;

private:
    int epoll_fd_;
    EventList events_;
    static const size_t kDefaultEventListSize;
    void update(int op, std::shared_ptr<Channel> channel);

public:
    explicit Epoller(EventLoop& loop);
    ~Epoller() override;
    TimePoint poll(int timeout_ms, ChannelList& active_channels) override;
    void update_channel(std::shared_ptr<Channel> channel) override;
    void remove_channel(std::shared_ptr<Channel> channel) override;
};



#endif