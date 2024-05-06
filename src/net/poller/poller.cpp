#include "poller.h"


Poller::Poller(Reactor& loop)
    : loop_(loop) {}

//判断某个channel是否属于某个poller
bool Poller::has_channel(Channel* channel) const {
    auto it = channels_.find(channel->get_fd());
    return it != channels_.end() && it->second == channel;
}