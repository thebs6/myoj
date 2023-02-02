#pragma once 

#include <vector>
#include <unordered_map>

#include "noncopyable.h"

class Channel;
class EventLoop;

class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    bool hasChannel(Channel *channel) const;
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;
};