#include "Socket.h"
#include "Logging.h"
#include <asm-generic/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/types.h>         

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    if(0 != bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)))
    {
        LOG_FATAL << "bind sockfd:" << sockfd_ << " fail";
    }
}
void Socket::listen()
{
    if(0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL << "listen sockfd:" << sockfd_ << " fail";
    }
}

int Socket::accept(InetAddress *peeraddr)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ::memset(&addr, 0, sizeof addr);
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }
    else
    {
        LOG_ERROR << "accept4() failed";
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR << "shutdownWrite error";
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof sockfd_);
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof sockfd_);  
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof sockfd_);  
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof sockfd_);  
}