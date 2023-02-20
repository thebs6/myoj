#include "TcpServer.h"
#include "Acceptor.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include "Logger.h"

#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

// 非空循环检测
static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is nullptr\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option)
    : loop_(CheckLoopNotNull(loop))
    , ipPort_(listenAddr.toIpPort())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)) // 用loop 和 listenAddr创建Accept
    , threadPool_(new EventLoopThreadPool(loop, name_)) // 用loop创建线程池
    , connectionCallback_()
    , messageCallback_()
    , started_(0)
    , nextConnId_(1)
{
    // 设置新连接回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)
    );
}

TcpServer::~TcpServer()
{   
    // 销毁所有连接
    for(auto &item : connections_)
    {
        // 这里item.second.reset()会导致item先释放嘛？ 
        //不会，这里拷贝到conn的时候，item和conn的计数器都加1，然后把item.second置空，conn的计数-1
        // 相当于把item move 给
        TcpConnectionPtr conn(item.second);
        // 调用智能指针的reset重置计数
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::start()
{
    // 开启TcpServer
    if(started_++ == 0)
    {
        // 开启threadPool_，里面的loop_会调用loop()函数开始事件循环
        threadPool_->start(threadInitCallback_);
        // 开启acceptor的监听
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

// 新连接回调
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    // sockfd 是accept返回的sockfd， peerAddr是accept的第二个参数，是连接上的sockaddr
    // 新连接命名 : "ipPort_-nextConnId_"
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from [%s]\n",
            name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    // 根据sockfd获取本地协议地址（IP地址和端口号）创建 localAddr
    sockaddr_in local;
    ::memset(&local, 0, sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
    {
        LOG_ERROR("getsockname error\n");
    }
    InetAddress localAddr(local);

    // 获取下一个线程(单线程：主线程， 多线程：子线程) 以下按多线程处理
    // 对connection的操作都是通过关联的EventLoop完成
    // 通过runInLoop把回调传入到EventLoop当中
    EventLoop* ioLoop = threadPool_->getNextLoop();
    // 创建新的conn对象关联子线程
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop, connName, sockfd, localAddr, peerAddr
    ));
    // 加入到connections_中
    connections_[connName] = conn;
    // 设置对应回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setMessageCallback(messageCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 子线程执行建立connectEstablished回调，connectEstablished会使能connectchannel_的读事件
    // 并且执行连接回调
    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    // 移除连接
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", 
            name_.c_str(), conn->name().c_str()
    );

    // 从connections_移除
    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}

