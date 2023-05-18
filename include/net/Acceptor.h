#pragma once

#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"
#include <functional>

class EventLoop;
class InetAddress;

// 用EventLoop Channel Poller封装一个accept得到Acceptor，
// socket, bind, listen, accept方法都在这
// 用于TcpServer的mainLoop中
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

    // mainLoop 和 Tcpserver一样   
    EventLoop* loop_;
    // acceptfd封装的socket和channel
    Socket  acceptSocket_;
    Channel acceptChannel_;
    // 新连接回调，一般需要把新连接放到子线程的subloop中
    NewConnectionCallback NewConnectionCallback_;
    // 是否在监听
    bool listening_;
};