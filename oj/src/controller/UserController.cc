#include "UserController.h"

void UserController::handleRequest(const HttpRequest& req, HttpResponse* resp) {
    if(req.getMethod() == HttpRequest::kPost && req.path() == "/shit") {
        handleRegister(req, resp);
    }
}


void UserController::handleRegister(const HttpRequest& req, HttpResponse* resp) {
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
