#include "Connector.h"
#include "Channel.h"
#include "Logger.h"
#include "Poller.h"
#include <asm-generic/errno.h>

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

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop), serverAddr_(serverAddr) {

}

Connector::~Connector() {
    if(!channel_) {
        LOG_ERROR("Connector::~Connector() channel != nullptr\n");
    }
}

void Connector::start() {
    connect_ = true;
    // loop_->runInLoop(
    //     [this]() {
    //         this->startInLoop();
    //     }
    // );
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    if(!loop_->isInLoopThread()) {
        return;
    }
    if(connect_) {
        connect();
    }
}

void Connector::stop() {
   if(!loop_->isInLoopThread()) {
        return;
    } 
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop() {
    if(!loop_->isInLoopThread()) {
        return;
    }
    if(state_ == kConnecting) {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect() {
    int sockfd = createNonblocking();
    const sockaddr* serverAddr = (sockaddr*)(serverAddr_.getSockAddr());
    int ret = ::connect(sockfd, serverAddr, 
    sizeof(*serverAddr));
    int savedErrno = (ret == 0) ? 0 : errno;
    if(savedErrno | 0 | EINPROGRESS | EINTR | EISCONN) {
        connecting(sockfd);
    } else if (savedErrno | EAGAIN | EADDRINUSE | EADDRNOTAVAIL | ENETUNREACH)
    {
        retry(sockfd);
    } else {
        ::close(sockfd);
    }
}

void Connector::restart() {
    if(!loop_->isInLoopThread()) {
        return ;
    }
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd) {
    setState(kConnecting);
    if(channel_) {
        LOG_ERROR("Connector::connecting");
        return;
    }

    channel_.reset(new Channel(loop_, sockfd));

    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite, this)
    );

    channel_->setErrorCallback(
        std::bind(&Connector::handleError, this)
    );

    /*
    FIXME: // channel_->tie(shared_from_this()); is not working,
            // as channel_ is not managed by shared_ptr
    */
    // channel_->tie(shared_from_this());
    channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queueInLoop(
        std::bind(&Connector::resetChannel, this)
    );
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}

void Connector::handleWrite() {
    LOG_INFO("Connector::handleWrite()");
    if(state_ != kConnecting) {
        LOG_ERROR("Connector::handleWrite state_ != kConnecting\n");
        return;
    }

    int sockfd = removeAndResetChannel();
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen)) {
        err = errno;
    } else {
        err = optval;
    }
    if(err) {
        retry(sockfd);
    } else {
        setState(kConnected);
        if(connect_) {
            newConnectionCallback(sockfd, serverAddress());
        } else {
            ::close(sockfd);
        }
    }
}

void Connector::handleError() {
    LOG_ERROR("Connector::handleErro\n");
    if(state_ == kConnecting) {
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::retry(int sockfd) {
    //TODO: 需要完成定时器之后继续完成
    LOG_ERROR("Connector::retry\n");
}