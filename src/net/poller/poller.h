#ifndef POLLER_H
#define POLLER_H

#include "channel.h"
#include <memory>
#include <unordered_map>
#include <vector>

class EventLoop;

class Poller {
public:
    using ChannelList = std::vector<Channel*>;
    using ChannelMap = std::unordered_map<int, Channel*>;

private:
    EventLoop& loop_;

protected:
    ChannelMap channels_;

public:
    explicit Poller(EventLoop& loop);
    virtual ~Poller() = default;
    Poller(const Poller& that) = delete;
    Poller& operator=(const Poller& that) = delete;
    virtual TimePoint poll(int timeout_ms,ChannelList& active_channels) = 0;
    virtual void update_channel(Channel* channel) = 0;
    virtual void remove_channel(Channel* channel) = 0;
    bool has_channel(Channel* channel) const;
};
#endif