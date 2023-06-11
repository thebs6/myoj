#include "AsyncLogging.h"
#include "LCServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"
#include "Logging.h"
#include "Timestamp.h"
#include "UserController.h"
#include "json.hpp"

using json = nlohmann::json;

void doTest(const HttpRequest& req, HttpResponse* resp) {
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

void doRegisterUser(const HttpRequest& req, HttpResponse* resp) {
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

static std::unordered_map<std::string, LCServer::HttpCallback> req_cb {
    {"/", doTest},
    {"/register", doRegisterUser}
};

void onRequest(const HttpRequest& req, HttpResponse* resp) {
    req_cb[req.path()](req, resp);
}

int main(int argc, char* argv[])
{
    std::cout << "LCServer pid = %d" << getpid() << std::endl;

    Logger::setLogLevel(Logger::LogLevel::DEBUG);

    EventLoop loop;
    LCServer server(&loop, InetAddress(8080), "http-server");
    server.setCPU(false);
    server.setThreadNum(0);
    // server.setHttpCallback(onRequest);
    UserController usc;
    server.registerController("/shit", &usc);
    server.start();
    loop.loop();
    // log.stop();
    return 0;
}