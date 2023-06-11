#pragma once
#include "ControllerBase.h"
using json = nlohmann::json;

class UserController : public ControllerBase {
public:
    UserController() = default;

    void handleRequest(const HttpRequest& req, HttpResponse* resp) override;

private:
    void handleRegister(const HttpRequest& req, HttpResponse* resp);
    // void handleLogin(const HttpRequest& req, HttpResponse* resp);
    // void handleTokenLogin(const HttpRequest& req, HttpResponse* resp);
    // void handleGetUserInfo(const HttpRequest& req, HttpResponse* resp);
    // void handleSubmitCode(const HttpRequest& req, HttpResponse* resp);
};
