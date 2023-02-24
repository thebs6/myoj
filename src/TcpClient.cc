#include "TcpClient.h"
#include "Callbacks.h"
#include "Connector.h"
#include "Poller.h"
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>

TcpClient::TcpClient(EventLoop* loop, 
              const InetAddress& serverAddr,
              const std::string& name) 
              : loop_(loop),
                connector_(new Connector(loop, serverAddr)),
                name_(name),
                connectionCallback_(),
                messageCallback_(),
                retry_(false),
                connect_(false),
                nextConnId_(1) 
{
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1, std::placeholders::_2)
    );
    LOG_INFO("TcpClient::TcpClient[%s], - connector", name.c_str());
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }

    auto cb = [this] (const TcpConnectionPtr& conn) -> void {
        loop_->queueInLoop([conn] {
            conn->connectDestroyed();
        });
    };

    if(conn) {
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    }

    if(unique) {
        // FIXME: muduo里面是强制关闭
        conn->shutdown();
    }
}



void TcpClient::newConnection(int sockfd, const InetAddress& peerAddr) {
    if(!loop_->isInLoopThread()) {
        LOG_ERROR(" TcpClient::newConnection \n");
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", 
    peerAddr.toIpPort().c_str(), nextConnId_);
    nextConnId_++;
    std::string connName = name_ + buf;

    sockaddr_in local;
    ::memset(&local, 0, sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
    {
        LOG_ERROR("getsockname error\n");
    }
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(loop_,
                                         connName,
                                         sockfd,
                                         localAddr,
                                         peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)
    );

    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    if(!loop_->isInLoopThread()) {
        LOG_ERROR(" TcpClient::newConnection \n");
        return;
    }

    if(loop_ != conn->getLoop()) {
        LOG_ERROR(" TcpClient::newConnection , line=%d,\n", __LINE__);
        return;
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(connection_ != conn) {
            LOG_ERROR(" TcpClient::newConnection , line=%d,\n", __LINE__);
        }
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if(retry_ && connect_) {
        LOG_INFO("TcpClient::connect [%s] - Reconnecting to %s",
        name_.c_str(), connector_->serverAddress().toIpPort().c_str());
        connector_->restart();
    }
}

void TcpClient::connect() {
    LOG_INFO("TcpClient::connect[%s], -connecting to [%s]\n", 
    name_.c_str(), connector_->serverAddress().toIpPort().c_str());
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(connect_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}
