#pragma once 

#include <functional>
#include <memory>
#include <thread>
#include <string>
#include <atomic>

#include "noncopyable.h"
// 封装线程
class Thread : noncopyable
{
public:
    //TODO 优化
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();
    
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string name() const { return name_; }
    static int numCreated() {return numCreated_; }

private:
    void setDefaultName();
    // 线程是否开始
    bool started_;
    // 是否joined
    bool joined_;
    // 线程
    std::shared_ptr<std::thread> thread_;
    // 线程id
    pid_t tid_;
    // 线程中执行的函数
    ThreadFunc func_;
    // 线程名
    std::string name_;
    // 用于设置默认名称的线程计数变量
    static std::atomic_int numCreated_;
};