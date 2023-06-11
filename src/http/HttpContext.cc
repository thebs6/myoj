#include "HttpContext.h"
#include <algorithm>


bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if(space != end && request_.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            const char* question = std::find(start, space, '?');
            if(question != space) {
                request_.setPath(start, question);
                request_.setQuery(question, space);
            } else {
                request_.setPath(start, space);
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end-1, "HTTP/1.");
            if(succeed) {
                if(*(end-1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if(*(end-1) == '0') {
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}


bool HttpContext::parseRequest(Buffer* buf, Timestamp receieveTime) {
    bool ok = true;
    bool hasMore = true;
    while(hasMore) {
        if(state_ == kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if(crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if(ok) {
                    request_.setReceiveTime(receieveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        }
        else if (state_ == kExpectHeaders)
        {
            const char* crlf = buf->findCRLF();
            if(crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if(colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    //FIXME: 空行，或者头的结尾
                    state_ = kExpectBody;
                    // hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore = false;
            }
        }
        else if (state_ == kExpectBody)
        {
            

            request_.setBody(std::string(buf->peek(), buf->readableBytes()));
            buf->retrieveAll();


            // LOG_DEBUG << body ;
            // 更新状态为已经处理完所有内容
            state_ = kGotAll;
            hasMore = false;
        }
    }
    return ok;
}
