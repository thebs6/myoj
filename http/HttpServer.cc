#include "HttpServer.h"

#include "Logger.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name,
               TcpServer::Option option)
               : server_(loop, listenAddr, name, option),
                 httpCallback_(defaultHttpCallback)
{
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1)
    );

    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
}

void HttpServer::start() {
    LOG_INFO("HttpServer [%s] starts listening on %s", 
             server_.name().c_str(), server_.ipPort().c_str());
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    if(conn->connected()) {
        // FIXME:
        HttpContext context;
        conn->setContext(&context);
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    HttpContext* context = static_cast<HttpContext*>(conn->getContext());
    if(!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 BadRequest\r\n\r\n");
    }

    if(context->getAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" || 
        (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    if(response.closeConnection()) {
        conn->shutdown();
    }
}