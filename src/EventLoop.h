#pragma once 

#include <functional>
#include <atomic>
#include <vector>
#include <mutex>
#include <memory>

#include "CurrentThread.h"
#include "Poller.h"
#include "Timestamp.h"
#include "noncopyable.h"

// 事件循环类， 协调控制Channel 和 Poller
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 用poller方法给Channel更新状态
    inline void updateChannel(Channel* channel) { poller_->updateChannel(channel); }
    inline void removeChannel(Channel* channel) { poller_->removeChannel(channel); }
    inline void hasChannel(Channel* channel) {poller_->hasChannel(channel); }

    inline bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    void wakeup();

private:
    void handleRead();
    void doPendingFunctors();
    
    // 循环开启退出标志
    std::atomic_bool looping_;
    std::atomic_bool quit_;

    // 线程id
    const pid_t threadId_;

    // poller的poll方法返回时间戳
    Timestamp pollReturnTime_;
    // 该事件循环中的唯一一个poller
    std::unique_ptr<Poller> poller_;

    /* 
        用于唤醒poller底层循环epoll_wait的eventfd 
        调用顺序: eventloop => loop() => poller->poll() => epoll_wait
    */
    int wakeupFd_;

    // 包装wakeupfd的channel
    std::unique_ptr<Channel> wakeupChannel_;

    // 回调执行标志位
    std::atomic_bool callingPendingFunctors_;

    // 回调方法队列
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
    
    // 管理的channel集合
    using ChannelList = std::vector<Channel*>;
    // 有事件发生的channel
    ChannelList activeChannels_;
};