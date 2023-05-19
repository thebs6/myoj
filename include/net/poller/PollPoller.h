#pragma once

#include <sys/poll.h>
#include <vector>

#include "Poller.h"
#include "Timestamp.h"

class PollPoller : public Poller
{
public:
    PollPoller(EventLoop* loop);
    ~PollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

    private:
    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    PollFdList pollfds_;
};