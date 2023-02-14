#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Acceptor.h"
#include "Logger.h"

// 创建非阻塞的sockfd
static int createNonblocking()
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create error\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop_, acceptSocket_.fd())
    , listening_(false)
{
    // sockfd设置为地址和端口可复用
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    // 绑定需要监听的InetAddress
    acceptSocket_.bindAddress(listenAddr);
    // 设置读回调
    acceptChannel_.setReaCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    // 关闭Channel
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    // 开始listen，使能channel的读事件
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

// 读回调，当listenfd上有事件可读，说明有新连接到来
void Acceptor::handleRead()
{   
    InetAddress peerAddr;
    // accept这个新连接
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0)
    {   
        // 调用新连接回调, 
        if(NewConnectionCallback_)
        {
            NewConnectionCallback_(connfd, peerAddr);
        }
        // 不处理新连接
        else
        {
            ::close(connfd);
        }
    }
    // accept失败
    else
    {
        LOG_ERROR("%s:%s:%d accept socket error : %d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        // fd 数量达到上限
        if(errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}
