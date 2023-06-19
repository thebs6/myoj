#pragma once

#include "HttpServer.h"
#include "json.hpp"
#include "Logging.h"

using Json = nlohmann::json;

class OJServer {
public:
    OJServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name
               ) : server_(loop, listenAddr, name) {}
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
    void run();
    void onRequest(const HttpRequest& req, HttpResponse* resp) ;

    void Post(std::string path, const HttpCallback& post_cb) {
        post_map_[path] = std::move(post_cb);
    }

    void Get(std::string path, const HttpCallback& get_cb) {
        get_map_[path] = std::move(get_cb);
    }

private:
    HttpServer server_;
    std::unordered_map<std::string, HttpCallback> post_map_;
    std::unordered_map<std::string, HttpCallback> get_map_;
};