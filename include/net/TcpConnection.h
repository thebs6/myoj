#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <any>
#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

// 使用EventLoop Channel Poller 封装一个Tcp连接, 一个tcp连接对应一个fd, 也就对应一个channel
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &nameArg,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    // 返回相关变量
    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    void send(const std::string& buf);
    void send(Buffer& buf);
    void shutdown();

    // 设置回调
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = std::move(cb); }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = std::move(cb); }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) 
    { 
        highWaterMarkCallback_ = std::move(cb); 
        highWaterMark_ = highWaterMark;
    }

    void connectEstablished();
    void connectDestroyed();

    void setContext(const std::any& context) {
        context_ = context;
    }

    std::any* getContext() {
        return &context_;
    }

private:
    // 连接状态枚举
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    void setState(StateE state) { state_ = state; }

    void shutdownInLoop();
    void sendInLoop(const void* data, size_t len);
    void sendInLoop(const std::string& message);
    
    void handleRead(Timestamp recieveTime);
    void handleWrite();
    void handleClose();
    void handleError();

     /* 
        对应的eventloop, 因为channel是由Eventloop管理的
        这里绝对不是baseLoop， 因为TcpConnection都是在subLoop里面管理的 
     */
    EventLoop *loop_;
    // 连接名称
    const std::string name_;
    // 连接状态
    std::atomic_int state_;
    //TODO ?干嘛用的
    bool reading_;

    // 一个tcp连接对应一个socket和channel
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    // 连接双方的InetAddress
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    // 连接的相关回调
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    // 高水位
    size_t highWaterMark_;

    // 读写缓冲区
    Buffer inputBuffer_;
    Buffer outputBuffer_; 

    // FIXME: void* => std::any ? 
    std::any context_;
};