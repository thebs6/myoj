#pragma once 

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallBack = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* loop, const std::string& name);
    ~EventLoopThreadPool();

    inline void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    
    void start(const ThreadInitCallBack &cb = ThreadInitCallBack());

    EventLoop *getNextLoop();
    std::vector<EventLoop*> getAllLoops();

    inline bool started() const { return started_; }
    inline const std::string name() const { return name_; }

private:
    EventLoop* baseloop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};