#include "AsyncLogging.h"
#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"
#include "Logging.h"
#include "Timestamp.h"
#include "json.hpp"

using json = nlohmann::json;

void onRequest(const HttpRequest& req, HttpResponse* resp) {
    if(req.getMethod() == HttpRequest::kPost && req.path() == "/") {
        json js;
        try {
            js = json::parse(req.body());
        } catch (json::exception) {
            resp->setStatusCode(HttpResponse::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/html");
            resp->addHeader("Server", "Muduo");
            std::string now = Timestamp::now().toFormattedString();
            resp->setBody("ERROR");
        }
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Muduo");
        std::string now = Timestamp::now().toFormattedString();
        resp->setBody(js.dump());
    }
}

int main(int argc, char* argv[])
{
    std::cout << "pid = %d" << getpid() << std::endl;

    Logger::setLogLevel(Logger::LogLevel::DEBUG);

    EventLoop loop;
    HttpServer server(&loop, InetAddress(8080), "http-server");
    server.setCPU(false);
    server.setThreadNum(0);
    server.setHttpCallback(onRequest);
    server.start();
    loop.loop();
    // log.stop();
    return 0;
}