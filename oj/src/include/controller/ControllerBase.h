#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
class ControllerBase {
public:
    virtual void handleRequest(const HttpRequest& req, HttpResponse* resp) = 0;
};