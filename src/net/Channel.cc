#include <memory>
#include <sys/epoll.h>

#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT ;

Channel::Channel(EventLoop *loop, int fd)
    : loop_ (loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , tied_(false)
{}

Channel::~Channel(){}

// channel可以封装在一个对象上，但是如果该对象先释放而channel的回调被设置的该对象上的方法就会出错
// 比如在TcpConnection中，将TcpConnection的成员函数设置为channel的回调，而如果TcpConnection被
// 销毁了，但是channel需要执行这些回调的时候就会出错
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

// 通过loop调用poller更新poller中的channel
// 流程： channel->update() => loop->updateChannel() => poller->updateChannel() => epoll_ctl()
void Channel::update()
{
    loop_->updateChannel(this);
}

// 在poller上移除该channel
void Channel::remove()
{
    loop_->removeChannel(this);
}

// 根据channel得到的revents_(从poller的epoll_wait中得到)执行对应回调
void Channel::handleEvent(Timestamp receiveTime)
{
    // 如果channel 使用过tied绑定一个对象
    if(tied_)
    {
        // 需要判断tied上的对象是否已经释放，这里通过提升弱智能指针判断
        std::shared_ptr<void> guard = tie_.lock();
        // 提升成功才执行真正的回调
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    // 没有tied 过对象
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 执行对应回调
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    // LOG_INFO << "channel handleEvent revents: %d\n", __FUNCTION__ ,revents_);
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closeCallback) closeCallback();
    }

    if(revents_ & EPOLLERR)
    {
        if(errorCallback) errorCallback();
    }

    if(revents_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallback) readCallback(receiveTime);
    }

    if(revents_ & EPOLLOUT) 
    {
        if(writeCallback) writeCallback();
    }
}


