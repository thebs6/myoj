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

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

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
    
    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_;

    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    std::atomic_bool callingPendingFunctors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;

    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
};