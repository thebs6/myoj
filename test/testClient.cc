#include "Callbacks.h"
#include "InetAddress.h"
#include "TcpClient.h"
#include "EventLoop.h"

void cb(const TcpConnectionPtr& newconn) {
    newconn->send("hello!!!!\n");
}



int main() {
    EventLoop loop;
    InetAddress serverAddr(8082);
    TcpClient tcpClient(&loop, serverAddr, "client");
    TcpConnectionPtr conn;
    tcpClient.setConnectionCallback(cb);

    tcpClient.setMessageCallBack(
        [] ( const TcpConnectionPtr & connection, Buffer* buf, Timestamp ts) {
            std::string recvMsg = buf->retrieveAllAsString();
            LOG_INFO("%s recvMsg: %s from %s \n", ts.toString().c_str(), recvMsg.c_str(), connection->peerAddress().toIpPort().c_str());
        }
    );
    tcpClient.connect();
    loop.loop();
}