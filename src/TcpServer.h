#pragma once

#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoop.h"
#include <atomic>
#include <memory>

class TcpServer : noncopyable
{
    using ThreadInitCallback = std::function<void(EventLoop *)>;

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

    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = std::move(cb);}
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = std::move(cb);}
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = std::move(cb);}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = std::move(cb);}

    inline void setThreadNum(int numThreads) { threadPool_->setThreadNum(numThreads); }

    void start();

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_;
    
    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_;
};