#pragma once

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

/*
    用Accept，TcpConnection封装一个TcpServer
    多线程的场景：mainLoop上跑Accept, 当Accept有新连接到来的时候把新连接轮询派发到subLoop（子线程）
    单线程：只有一个loop，一起跑
*/ 

class TcpServer : noncopyable
{
public:
    //TODO 优化
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    // option枚举
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option = kNoReusePort);
    ~TcpServer();

    // 设置回调
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = std::move(cb);}
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = std::move(cb);}
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = std::move(cb);}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = std::move(cb);}

    // 设置线程数量, 不设置就是单线程
    inline void setThreadNum(int numThreads) { threadPool_->setThreadNum(numThreads); }

    void start();

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    // 主线程的主循环 mainloop
    EventLoop *loop_;
    
    // 服务端的ip Port name_
    const std::string ipPort_;
    const std::string name_;

    // 管理的Acceptor
    std::unique_ptr<Acceptor> acceptor_;

    // 线程池
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    
    // 新连接回调，设置给Acceptor
    ConnectionCallback connectionCallback_;
    // 消息回调，设置给TcpConnection的底层读回调
    MessageCallback messageCallback_;
    // 写完成回调，设置给TcpConnection的写完成回调
    WriteCompleteCallback writeCompleteCallback_;

    // 线程初始化函数
    ThreadInitCallback threadInitCallback_;

    // 开始计数，用于保证只执行一个线程和一个循环
    std::atomic_int started_;

    // 下一个线程
    int nextConnId_;

    // 管理的连接 key = 线程名称 ,value = TcpConnectionPtr
    ConnectionMap connections_;
};