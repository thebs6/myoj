#include "TcpServer.h"
#include "Logger.h"

#include <string>
#include <functional>

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
        : loop_(loop), server_(loop, addr, name)
    {
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
        );
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2,  std::placeholders::_3)
        );
    }  

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)   
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
    }

private:
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    InetAddress addr(8082);
    EventLoop loop;
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();
}