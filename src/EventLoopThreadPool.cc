

#include "EventLoopThread.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, const std::string& nameArg)
    : baseloop_(loop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}

// 开启线程池
void EventLoopThreadPool::start(const ThreadInitCallBack &cb)
{
    started_ = true;
    // 循环开启numThreads_个子线程
    for(int i = 0; i < numThreads_; i++)
    {   
        // 设定子线程的名称: “线程池名称线程索引”
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        // 给每个子线程传递ThreadInitCallBack
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // 运行子线程，返回子线程的eventloop指针
        loops_.push_back(t->startLoop());
    }

    // 只有一个线程
    if(numThreads_ == 0 && cb)
    {
        cb(baseloop_);
    }
}

// 获取下一个EventLoop，采用轮询算法
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseloop_;
    // 如果loops为空，说明只有一个线程，直接返回baseloop_
    if(!loops_.empty())
    {
        // 循环获取下一个EventLoop
        loop = loops_[next_++];
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }   
    return loop;
}

// 获取线程池中所有的EventLoop*
std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1,baseloop_);
    }
    else
    {
        return loops_;
    }
}