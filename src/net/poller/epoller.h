#ifndef EPOLLER_H
#define EPOLLER_H

#include "channel.h"
#include "logger.h"
#include "poller.h"
#include "sys/epoll.h"
#include "timer.h"
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <unistd.h>

class Epoller : public Poller {
public:
    using EventList = std::vector<epoll_event>;
    enum ChannelState { New = -1, Added = 1, Deleted = 2 };

private:
    int epoll_fd_;
    EventList events_;
    static const size_t kDefaultEventListSize;
    void update(int op, Channel* channel);
    void fill_channels(int evnets_cnt, ChannelList& active_channels);

public:
    explicit Epoller(EventLoop& loop);
    ~Epoller() override;
    TimePoint poll(int timeout_ms, ChannelList& active_channels) override;
    void update_channel(Channel* channel) override;
    void remove_channel(Channel* channel) override;
};



#endif