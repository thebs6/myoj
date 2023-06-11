#pragma once 

#include <vector>
#include <unordered_map>

#include "ThreadLoadCounter.h"
#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

// Poller基类， 暂时只实现epollPoller
class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannelList) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;
    static Poller* newDefaultPoller(EventLoop *loop);

    virtual int load() = 0;
protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    // 管理的Channel
    ChannelMap channels_;
    ThreadLoadCounter loadCountor_;
private:
    EventLoop *ownerLoop_;
};