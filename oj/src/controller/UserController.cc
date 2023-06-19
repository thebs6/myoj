#include "UserController.h"
#include "UserService.h"

void UserController::handleRequest(const HttpRequest& req, HttpResponse* resp) {
    Json resJson;
    Json reqJson;
    try {
        reqJson = Json::parse(req.body());
        if(req.getMethod() == HttpRequest::kPost && req.path() == "/shit") {
            Json resJson = UserService::instance()->register(reqJson);
        }
    } catch (Json::exception) {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Muduo");
        resp->setBody("ERROR");
    }
    
}


Json UserController::handleRegister(const Json& req) {
    
    UserService::instance()->register()
}
