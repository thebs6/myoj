#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include "InetAddress.h"
#include "TcpServer.h"

void cb(const HttpRequest& req, HttpResponse*) {
    LOG_INFO("a new HttpRequest!\n");
}

int main(int argc, const char** argv) {
    EventLoop loop;
    InetAddress addr(8082);
    HttpServer server(&loop, addr, "http");
    server.setHttpCallback(cb);
    server.start();
    loop.loop();
    return 0;
}