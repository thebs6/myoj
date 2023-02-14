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
    // 退出循环
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        // 回收线程资源
        thread_.join();
    }
}

// 开启一个线程，这个线程里面有一个循环，并返回该循环的指针
EventLoop* EventLoopThread::startLoop()
{
    // 开启一个线程，该线程执行threadFunc方法
    thread_.start();

    // 使用条件变量使主线程等待子线程返回一个loop
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
    // 如果有初始化线程函数则执行, 用来初始化loop
    if(initCallback_)
    {
        initCallback_(&loop);
    }

    // 加锁的原因：因为startLoop会多次调用，而这里需要用loop_传递指针，多个线程可能会同时对
    // loop_进行读写
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        // 通知主线程获取loop_
        cond_.notify_one();
    }
    // 开启循环
    loop.loop();
    // 当循环退出的时候loop_ 置空
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}