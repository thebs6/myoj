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
    : Poller(loop) // 调用基类构造函数
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC)) // 带标志位的epoll_create
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

// poll方法包装一个epoll_wait
Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels) 
{
    // poll会被频繁调用，用LOG_DEBUG输出，当并发量高的时候关闭LOG_DEBUG
    LOG_DEBUG("func=%s => fd total count: %lu\n", __FUNCTION__, channels_.size());

    // 这里要 &*events_.begin() 而不是用events_begin(), 因为begin()返回的是一个iterator类型而不是指针类型，虽然用*可以解引用。
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int errorTmp = errno;
    // 最后返回的时间戳
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        LOG_DEBUG("func=%s => %d events happend\n", __FUNCTION__, numEvents);
        // 将epoll_wait返回的事件填充到activeChannels上
        fillActiveChannels(numEvents, activeChannels);
        // 如果events_中全都活跃，那么需要对events_扩容
        if(numEvents == events_.size()) 
        {
            events_.resize(2 * events_.size());
        }
    }
    // 无事件发生，epoll_wait超时
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout\n", __FUNCTION__);
    }
    // 出现异常
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

// 填充活跃事件集合
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    /*
        对每个活跃的channel设置他的返回事件revents，插入到activeChannels
        events_[i].data.ptr 就是 channel*， 原因见  EpollPoller::update 
    */
    for(int i = 0; i < numEvents; i++) {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

// 用于更新Channel的状态，对于不在该poller管理的channels_需要添加进channels_
void EpollPoller::updateChannel(Channel *channel) 
{
    // channels_ 的 key = fd
    // chanenls_ 的 value = channel*
    const int index = channel->index();
    LOG_DEBUG("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    // 对于新的channel和已删除的channel需要添加到channels_
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
        // 更改channel状态
        channel->set_index(kAdded);
        // 真正的更新是调用底层的epoll_ctl
        update(EPOLL_CTL_ADD, channel);
    } 
    // 已存在的channel处理：
    else
    {   
        int fd = channel->fd();
        // 如果channel不关心任何事件就把他移除
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channels_.erase(fd);
        }
        // 修改channel对事件的状态
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }

}

// 从channels_移除channel
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

// 封装epoll_ctl，根据operation操作channel对应的fd
void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof event);

    // 就是在这里对把event和channel，fd关联起来
    /*
        struct epoll_event
        {
            uint32_t events;	
            epoll_data_t data;	
        } 

        union epoll_data
        {
            void *ptr;
            int fd;
            uint32_t u32;
            uint64_t u64;
        } epoll_data_t;
        event.data.fd = fd , event.data.ptr = channel*
    */
    int fd = channel->fd();
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    // 调用epoll_ctl修改fd
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
