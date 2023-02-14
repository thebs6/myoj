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
    , highWaterMark_(64 * 1024 * 1024)
{
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

void TcpConnection::send(const std::string& buf)
{
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
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = 0;
    bool faultError = false;
    if(state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing!\n");
    }

    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else
        {
            nwrote = 0;
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
    if(!faultError && remaining > 0)
    {
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
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    if(connectionCallback_) connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        if(connectionCallback_) connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this())
                    );
                }
                if(state_ == kDisconnected)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite\n");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
    }
}

void TcpConnection::handleRead(Timestamp recieveTime)
{
    int saveErrno = 0;
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


