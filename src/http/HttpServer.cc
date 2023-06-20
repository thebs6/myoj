#include "HttpServer.h"

#include "Buffer.h"
#include "CurrentThread.h"
#include "Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <memory>

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    LOG_INFO << "NOT FOUND";
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name,
               TcpServer::Option option)
               : server_(loop, listenAddr, name, option),
                 httpCallback_(defaultHttpCallback), pool(1)
{
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1)
    );

    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
}

void HttpServer::start() {
    LOG_INFO << "HttpServer [" << server_.name() << " ] starts listening on " << server_.ipPort();
    server_.start();
    pool.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    if(conn->connected()) {
        conn->setContext(HttpContext());
    }
}

// void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
//     HttpContext* context = std::any_cast<HttpContext>(conn->getContext());

//     pool.submitTask(pool.hash(conn->peerAddress().toIpPort()), [&, conn]() {
//         Buffer readBuf;
//         readBuf.swap(*buf);
//         LOG_DEBUG << readBuf.peek();

//         if(!context->parseRequest(&readBuf, receiveTime)) {
//             conn->send("HTTP/1.1 400 BadRequest\r\n\r\n");
//         }
//         if(context->getAll()) {
//             onRequest(conn, context->request());
//             context->reset();
//         }
//     });

    
// }

void HttpServer::handleRequest(const TcpConnectionPtr& conn, std::shared_ptr<Buffer> buf, Timestamp receiveTime, HttpContext* context) {
    
    LOG_DEBUG << "  " << "连接" << conn->peerAddress().toIpPort() << "分配到" << CurrentThread::tid() ;

    if (!context->parseRequest(buf.get(), receiveTime)) {
        conn->send("HTTP/1.1 400 BadRequest\r\n\r\n");
    }
    if (context->getAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    HttpContext* context = std::any_cast<HttpContext>(conn->getContext());
    
    // auto readBuf = std::make_shared<Buffer>();

    // readBuf->swap(*buf);
    // LOG_DEBUG << "   [" << CurrentThread::tid() << "]" <<" onMessage submitTask" <<  readBuf->peek();
    // // pool.submitTask(idx, [this, conn, readBuf, receiveTime, context]() {
    // handleRequest(conn, readBuf, receiveTime, context);
    if (!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 BadRequest\r\n\r\n");
    }
    if (context->getAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
    // });
}


void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" || 
        (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    auto buf = std::make_shared<Buffer>();
    response.appendToBuffer(buf.get());
    LOG_DEBUG << "   [" << CurrentThread::tid() <<"] onRequest -> send ->" << buf->readableBytes() << " bytes" ; 
    // LOG_DEBUG << buf.peek();
    LOG_DEBUG << "  [" << CurrentThread::tid() << "]" << "HttpServer::onRequest" << " before send";
    conn->send(buf);
    LOG_DEBUG << "  [" << CurrentThread::tid() << "]" << "HttpServer::onRequest" << " after send";
    if(response.closeConnection()) {
        conn->shutdown();
    }
    LOG_DEBUG << "  [" << CurrentThread::tid() << "]" << "HttpServer::onRequest" << " out";
}