#pragma once 

#include <functional>
#include "EventLoop.h"
#include "Thread.h"
#include <condition_variable>

class EventLoopThread
{
public:

    using ThreadInitCallBack = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallBack &initCallback = ThreadInitCallBack(),
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallBack initCallback_;
};