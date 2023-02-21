#pragma once
#include <atomic>
#include <functional>

#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoop.h"
#include "Callbacks.h"

class Connector : noncopyable, std::enable_shared_from_this<TcpConnection>
{
public:
    using NewConnectionCallback = std::function<void(int sockfd,const InetAddress& peerAddr)>;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();
    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback = std::move(cb); }

    void start();
    void restart();
    void stop();
    const InetAddress& serverAddress() const { return serverAddr_; }
private:
    enum States {kDisconnected, kConnecting, kConnected };

    void setState(States s) {state_ = s;}
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();


    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;


    EventLoop* loop_;
    InetAddress serverAddr_;
    std::atomic_bool connect_;
    std::atomic<States> state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback;
    int retryDelayMs_;
};