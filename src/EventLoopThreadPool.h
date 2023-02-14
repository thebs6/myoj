#pragma once 

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

// 封装线程池
class EventLoopThreadPool : noncopyable
{
public:
    //TODO 函数指针
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
    // 主线程
    EventLoop* baseloop_;
    // 线程池的名称
    std::string name_;
    // 开启标志位
    bool started_;
    // 线程数量
    int numThreads_;
    // 下一个线程的的索引
    int next_;
    // 所有线程指针的集合
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    // 所以事件循环的集合
    std::vector<EventLoop*> loops_;
};