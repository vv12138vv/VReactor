#include "epoller.h"
#include "logger.h"
#include <asm-generic/errno-base.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>


const size_t Epoller::kDefaultEventListSize = 16;


Epoller::Epoller(Reactor& loop)
    : Poller(loop)
    , epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
    , events_(kDefaultEventListSize) {
    if (epoll_fd_ < 0) {
        LOG_FATAL << "epoll_create1() fatal:" << errno;
    }
}

Epoller::~Epoller() {
    close(epoll_fd_);
}

//核心函数，其内部不断调用epoll_wait获取事件
TimePoint Epoller::poll(int timeout_ms, ChannelList& active_channels) {
    size_t events_cnt = epoll_wait(epoll_fd_, &(*events_.begin()), static_cast<int>(events_.size()), timeout_ms);
    int save_errno = errno;
    TimePoint now = std::chrono::time_point_cast<Duration>(BaseClock::now());
    if (events_cnt > 0) {
        fill_channels(events_cnt, active_channels);
        if (events_cnt == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (events_cnt == 0) {
        LOG_DEBUG << "epool timeout";
    } else {
        if (save_errno != EINTR) {
            errno = save_errno;
            LOG_ERROR << "EPoller::poll failed";
        }
    }
    return now;
}

void Epoller::fill_channels(int events_cnt, ChannelList& active_channels) {
    for (int i = 0; i < events_cnt; i += 1) {
        auto channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_real_event(events_[i].events);
        active_channels.push_back(channel);
    }
}

void Epoller::update(int op, Channel* channel) {
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    int fd = channel->get_fd();
    event.events = channel->get_event();
    event.data.fd = fd;
    event.data.ptr = channel;
    //修改epoll_ctl
    if (epoll_ctl(epoll_fd_, op, fd, &event) == -1) {
        if (op == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl() del error:" << errno;
        } else {
            LOG_FATAL << "epoll_ctl() add/mod error:" << errno;
        }
    }
}

//更新channel在epoll上的状态
void Epoller::update_channel(Channel* channel) {
    const int idx = channel->get_idx();
    if (idx == ChannelState::New || idx == ChannelState::Deleted) {   //判断channel是新的未被监视事件、正被监视或已不再监视
        int fd = channel->get_fd();
        if (idx == ChannelState::New) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {   // idx==ChannelState::deleted
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_idx(ChannelState::Deleted);
        update(EPOLL_CTL_ADD, channel);   //将事件注册到内核中
    } else {
        int fd = channel->get_fd();
        //一些断言检查
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(idx == ChannelState::Added);
        //若该channel无处理事件了，代表要从epoll中注销了
        if (channel->is_none_event()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_idx(ChannelState::Deleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

//移除对channel的监视
void Epoller::remove_channel(Channel* channel) {
    int fd = channel->get_fd();

    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->is_none_event());

    int idx = channel->get_idx();
    assert(idx == ChannelState::Added || idx == ChannelState::Deleted);
    size_t ret = channels_.erase(idx);
    assert(ret == 1);

    if (idx == ChannelState::Added) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_idx(ChannelState::New);
}