#include "epoller.h"
#include "timer.h"
#include <chrono>
#include <sys/epoll.h>

const size_t Epoller::kDefaultEventListSize = 16;


Epoller::Epoller(EventLoop& loop)
    : Poller(loop)
    , epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
    , events_(kDefaultEventListSize) {
    if (epoll_fd_ < 0) {
        // log
    }
}

Epoller::~Epoller() {
    close(epoll_fd_);
}

TimePoint Epoller::poll(int timeout_ms, ChannelList& active_channels) {
    size_t events_cnt = epoll_wait(epoll_fd_, &(*events_.begin()), static_cast<int>(events_.size()), timeout_ms);
    TimePoint now=std::chrono::time_point_cast<Duration>(BaseClock::now());


}