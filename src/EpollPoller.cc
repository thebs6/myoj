#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

#include "EpollPoller.h"
#include "Logger.h"
#include "Timestamp.h"
#include "Channel.h"

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error : %d \n", errno);
    }
}

EpollPoller::~EpollPoller()
{
    close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels) 
{
    // poll会被频繁调用，用LOG_DEBUG输出，当并发量高的时候关闭LOG_DEBUG
    LOG_DEBUG("func=%s => fd total count: %lu\n", __FUNCTION__, channels_.size());

    // 这里要 &*events_.begin() 而不是用events_begin(), 因为begin()返回的是一个iterator类型而不是指针类型，虽然用*可以解引用。
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int errorTmp = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        LOG_DEBUG("func=%s => %d events happend\n", __FUNCTION__, numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size()) 
        {
            events_.resize(2 * events_.size());
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout\n", __FUNCTION__);
    }
    else
    {
        if(errorTmp != EINTR)
        {
            errno = errorTmp;
            LOG_ERROR("%s error", __FUNCTION__);
        }
    }
    return now;
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvents; i++) {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel *channel) 
{
    const int index = channel->index();
    LOG_DEBUG("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted)
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        else
        {
            
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } 
    else
    {
        int fd = channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channels_.erase(fd);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }

}

void EpollPoller::removeChannel(Channel *channel) 
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof event);

    int fd = channel->fd();
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("func=%s => epoll_del error\n", __FUNCTION__);
        }
        else
        {
            LOG_ERROR("func=%s => epoll_mod / epoll_add error\n", __FUNCTION__);
        }
    }
}
