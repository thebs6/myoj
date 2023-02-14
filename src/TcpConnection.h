#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &nameArg,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    void send(const std::string& buf);
    void shutdown();

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

private:
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
    
    void handleRead(Timestamp recieveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop *loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_; 
};