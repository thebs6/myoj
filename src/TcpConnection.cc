#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <errno.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>


static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is nullptr\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop))
    , name_(nameArg)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024) // 高水位初始化64M
    , context_(nullptr)
{
    // 设置对应回调
    channel_->setReaCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d, state = %d \n", 
    name_.c_str(), channel_->fd(), static_cast<int>(state_));
}

// 发送数据
void TcpConnection::send(const std::string& buf)
{
    // 必须处于已连接状态才能发送消息
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size())
            );
        }
        /*
            TODO 上面可以直接优化成一句loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size())
            );
        */ 
    }
    // TODO 待确定： 其实这里肯定是走runInloop的，以为TcpConnection和loop是在一个线程上的
}

// 真正的发送
void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = 0;
    bool faultError = false;
    // 已断开则不能发送
    if(state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing!\n");
    }
    // 如果channel没有注册可写事件，说明channel是第一次写
    // 且输出缓冲区中没有数据需要发送， 说明之前还没发送的信息已经全部发送出去了
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        // 向fd写len个数据
        nwrote = ::write(channel_->fd(), data, len);
        if(nwrote >= 0)
        {
            // 计算剩余未写数据
            remaining = len - nwrote;
            // 如果全部写完并且有写完成事件回调
            if(remaining == 0 && writeCompleteCallback_)
            {
                //TODO 为什么这里写完成事件回调要用queueInLoop？
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else
        {   
            // 写入异常，将nwrote=0
            nwrote = 0;
            // 如果不是缓冲区已满而是连接重置或者关闭则faultError = true;
            if(errno != EWOULDBLOCK)
            {
                LOG_ERROR("Tcpconnection::sendInLoop\n");
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }

        }
    }   
    // 如果没有出错 并且 还有数据没有发送
    if(!faultError && remaining > 0)
    {
        /* 
            这里显示出了outputBuffer_作用：
            当不能通过系统调用write所有数据的时候，把剩下的数据放到outputBuffer_中
            当outputBuffer_中的原来的数据没有超过高水位 
            并且 
            本次加进来的数据使得outputBuffer_超过高水位了
            就把高水位的回调函数加到循环的回调队列
            然后把本次剩下的数据加到outputBuffer_中，注册channel的写事件，使得下次可以继续写
        */
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_ && oldlen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(
                highWaterMarkCallback_, shared_from_this(), oldlen + remaining
            ));
        }
        outputBuffer_.append((char*)data + nwrote, remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

// 断开连接执行断开回调
void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, shared_from_this())
        );
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting()) // 说明outputBuffer中的数据已经全部发送完成
    {
        socket_->shutdownWrite(); // 关闭写端
    }
}

// 连接建立回调
void TcpConnection::connectEstablished()
{
    // 设置状态
    setState(kConnected);
    // 将channel tie 到当前连接，因为channel要调用连接的回调
    channel_->tie(shared_from_this());
    // 使能读端，接收客户端发送的数据
    channel_->enableReading();
    // 执行连接回调
    if(connectionCallback_) connectionCallback_(shared_from_this());
}

// 断开连接
void TcpConnection::connectDestroyed()
{
    // 
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        // 把channel的所有感兴趣的事件，从poller中del掉
        channel_->disableAll(); 
        if(connectionCallback_) connectionCallback_(shared_from_this());
    }
    // 从poller中移除channel
    channel_->remove();
}

// 写事件回调，发送数据的时候没有发送完数据然后注册了写事件
void TcpConnection::handleWrite()
{
    // channel注册了写事件
    if(channel_->isWriting())
    {
        int saveErrno = 0;
        // 尝试把outputBuffer_的数据都写入
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if(n > 0)
        {
            // 写入了n个数据，outputBuffer_需要去掉n个数据
            outputBuffer_.retrieve(n);
            // 如果outputBuffer_全写完了
            if(outputBuffer_.readableBytes() == 0)
            {
                // 取消channel注册的写事件
                channel_->disableWriting();
                // 调用写完回调
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this())
                    );
                }
                // 如果连接断开
                if(state_ == kDisconnected)
                {
                    shutdownInLoop();
                }
            }
        }
        else // 这里严谨点
        {
            LOG_ERROR("TcpConnection::handleWrite\n");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
    }
}

// 读回调
void TcpConnection::handleRead(Timestamp recieveTime)
{
    int saveErrno = 0;
    // 因为系统的读缓冲一般比64M小，所以没有考虑读不完
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
    {
        if(messageCallback_)
        {
            messageCallback_(shared_from_this(), &inputBuffer_, recieveTime);
        }
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead fd = %d error\n", channel_->fd())
        handleError();
    }
}

// 关闭回调， 也会执行连接回调，所以连接回调是在连接和关闭以及销毁都会执行
void TcpConnection::handleClose()
{
    LOG_INFO("Tcpconnection::handleClose fd=%d, state=%d\n",
              channel_->fd(), static_cast<int>(state_));
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr connptr(shared_from_this());
    if(connectionCallback_) 
    {
        connectionCallback_(connptr);
    }
    if(closeCallback_)
    {
        closeCallback_(connptr);
    }
}

// 错误回调
void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen))
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}


