#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <memory>
#include <sys/eventfd.h>

#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"

// __thread 使得每个线程都有一份该全局变量的拷贝
// t_loopInThisThread 用于保证每个线程只有一个loop
__thread EventLoop *t_loopInThisThread = nullptr;

// poll方法超时事件
const int kPollTimeMs = 10000;

// 创建非阻塞eventfd
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error: %d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid()) // 设置线程id
    , poller_(Poller::newDefaultPoller(this)) // 初始化poller
    , wakeupFd_(createEventfd()) // 创建wakeupfd
    , wakeupChannel_(new Channel(this, wakeupFd_)) // 用wakeupfd初始化wakeupChannel
    , callingPendingFunctors_(false) 
{
    LOG_DEBUG("EventLoop create %p in thread %d\n", this, threadId_);
    // 如果当前线程已经有loop存在就报异常
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设定wakeupChannel的回调
    wakeupChannel_->setReaCallback(std::bind(&EventLoop::handleRead, this));
    // 注册wakeupChannel读事件
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    // 移除wakeupChannel
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// 事件循环开始
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n", this);

    while(!quit_)
    {
        // 清楚上一次有事件发生的Channel
        activeChannels_.clear();
        // 传入activeChannels_到poller的poll方法中，等待其中的事件发生
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        // 对所有有事件发生的Channel执行对应的回调方法
        for (Channel* channel : activeChannels_) {
            channel->handleEvent(pollReturnTime_);
        }
        // 执行该循环的回调
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping.\n", this);
    looping_ = false;
}

// 退出循环
void EventLoop::quit()
{
    quit_ = true;
    // 其他线程调用该线程的退出时需要唤醒该线程
    if (!isInLoopThread())
    {
        wakeup();
    }
}

// 在该循环中执行回调方法
void EventLoop::runInLoop(Functor cb)
{
    // 如果是当前线程调用该方法则直接执行
    if(isInLoopThread())
    {
        cb();
    }
    // 否则添加到该循环的回调队列中
    else 
    {
        queueInLoop(cb);
    }
}

// 添加回调到当前循环的回调队列中
void EventLoop::queueInLoop(Functor cb)
{
    // 这里需要加锁， 因为会有多个线程同时读写loop中的pendingFunctors_
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // emplace_back 防止了push_back中的拷贝构造， 直接move进vector中
        pendingFunctors_.emplace_back(cb);
    }

    // 如果其他线程调用该线程的退出时需要唤醒该线程
    // 如果callingPendingFunctors_为true 说明此时正在执行回调，所以也需要wakeup，从而使得上一次回调队列中的回调
    // 执行完可以马上执行新添加的回调
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

// wakeupChannel 的读事件：读一个数据，如果不读出来的话在LT模式下这个这个事件会一直触发
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("%s reads %lu bytes instead of 8\n", __FUNCTION__,n);
    }
}

// 唤醒事件循环，往wakeupfd里面写一个数据，从而触发注册在epoll上的读事件，跳出epoll_wait
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("%s writes %lu bytes instead of 8\n", __FUNCTION__,n);
    }
}

// 执行队列上的回调函数
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    // 对pendingFunctors_加锁， 因为其他线程会读写pendingFunctors_
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 用swap，直接把全部回调取出来执行而不是在临界区执行，提高效率
        functors.swap(pendingFunctors_);
    }

    // 引用减少拷贝开销
    for(const Functor &functor : functors)
    {
        functor();
    }

    callingPendingFunctors_ = false;
}


