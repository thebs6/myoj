#pragma once

#include "Callbacks.h"
#include "noncopyable.h"
#include "EventLoop.h"
class TcpClient : noncopyable {
public:

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    // TcpConnectionPtr 
};