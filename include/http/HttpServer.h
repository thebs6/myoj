#pragma once

#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpServer.h"
class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable {
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name,
               TcpServer::Option option = TcpServer::kNoReusePort);

    EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb) {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void setCPU(bool on) {
        server_.enable_cpu_affinity(on);
    }

    void start();

private:

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest&);

    TcpServer server_;
    HttpCallback httpCallback_;
};