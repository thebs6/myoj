#include "Thread.h"
#include "CurrentThread.h"
#include <memory>
#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    // 设置线程默认名称
    setDefaultName();
}

Thread::~Thread()
{
    // 线程开始且不可join的时候把线程detach掉，让系统回收资源
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

// 开启线程
void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));

    // 这里要等待tid_ 已经返回，保证调用start方法之后可以正常访问tid_
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

// 设置线程默认名称， 这里应该有问题，当构造函数中传了其他名称这里就不会调用
// 那么这里计算出来的numCreated_没有任何意义，只能是设置名称而不是所有线程个数
void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}