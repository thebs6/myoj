#include "EventLoopThread.h"
#include "EventLoop.h"
#include <memory>


/*
* EventLoopThread 构造函数 => 初始化Thread对象，传入线程中loop执行的initCallback方法（可选）
* startLoop() => Thread.start() => 主线程 使用条件变量等待loop_                                             => 被唤醒 => 将loop_返回
*                               => 子线程 创建一个loop，执行initCallback方法, 获取锁并loop_ = loop,唤醒主线程。 loop.loop()开始循环，结束循环后将loop_=nullptr
*/                                         

EventLoopThread::EventLoopThread(const ThreadInitCallBack &initCallback,
                                 const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , mutex_()
    , cond_()
    , initCallback_(initCallback)
{
}
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if(initCallback_)
    {
        initCallback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}