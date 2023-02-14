#pragma once

#include <sys/epoll.h>
#include <vector>

#include "Poller.h"
#include "Timestamp.h"

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
    
private:
    // epoll_wait 监听的时间数量
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    // epoll fd
    int epollfd_;

    // 监听的事件集合
    EventList events_;
};
