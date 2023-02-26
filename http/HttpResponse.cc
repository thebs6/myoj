#include "HttpResponse.h"
#include <cstdio>
#include <sstream>

void HttpResponse::appendToBuffer(Buffer* output) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_.c_str());
    output->append("\r\n");

    if(closeConnection_) {
        output->append("Connection: close\r\n");
    } else {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
        output->append("Connection: keep-Alive\r\n");
    }

    for(const auto& header : headers_) {
        std::stringstream s;
        s << header.first << ": "<< header.second << "\r\n";
        output->append(s.str().c_str());
    }

    output->append("\r\n");
    output->append(body_.c_str());
}