#pragma once

#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"
#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    inline void setNewConnectionCallback(const NewConnectionCallback& cb) { NewConnectionCallback_ = std::move(cb); }
    
    void listen();
    inline bool listening() { return listening_; }
private:
    void handleRead();
    
    EventLoop* loop_;
    Socket  acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback NewConnectionCallback_;
    bool listening_;
};