#pragma once

#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoop.h"
#include "Connector.h"
#include "TcpConnection.h"
#include "Logger.h"

#include <memory>
#include <mutex>
#include <string>
class TcpClient : noncopyable {
public:
    using ConnectorPtr = std::shared_ptr<Connector> ;
    TcpClient(EventLoop* loop, 
              const InetAddress& serverAddr,
              const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();
    TcpConnectionPtr connection() {
        std::unique_lock<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* getLoop() { return loop_ ;}
    bool getRetry() { return retry_; }
    void enableRetry() { retry_ = true; }
    const std::string& name() const { return name_ ;}
    void setConnectionCallback(ConnectionCallback cb) {connectionCallback_ = std::move(cb);}
    void setMessageCallBack(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompeleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }


private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;
    bool connect_;
    int nextConnId_;
    std::mutex mutex_;
    TcpConnectionPtr connection_;
};