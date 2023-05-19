#include "PollPoller.h"
#include "Poller.h"
#include "EpollPoller.h"
#include <cstdlib>
Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return new PollPoller(loop);
    }
    else
    {
        return new EpollPoller(loop);
    }
}