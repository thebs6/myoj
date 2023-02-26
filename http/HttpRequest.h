#pragma once

#include "Logger.h"
#include "Timestamp.h"
#include <unordered_map>
#include <utility>

class HttpRequest {
public:
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };
    
    enum Version
    {
        kUnknown, kHttp10, kHttp11
    };

    static const std::unordered_map<std::string, Method> kStringToHttpMethod;

    HttpRequest() : method_(kInvalid), version_(kUnknown) {}

    void setVersion(Version v) {
        version_ = v;
    }

    Version getVersion() const {
        return version_;
    }

    void setMethod(Method m) {
        method_ = m;
    }

    bool setMethod(const char* start, const char* end) {
        if(method_ != kInvalid) {
            LOG_ERROR("setMethod Error\n");
            return false;
        }
        const std::string m(start, end);
        
        auto it = kStringToHttpMethod.find(m);
        if(it != kStringToHttpMethod.end()) {
            method_ = it->second;
            return true;
        }
        method_ = kInvalid;
        return false;
    }

    Method getMethod() const {
        return method_;
    }

    const char* methodString(Method method) const {
        for(auto& it : kStringToHttpMethod) {
            if(it.second == method) {
                return it.first.c_str();
            }
        }
        return "UNKNOWN";
    }

    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }

    const std::string& path() const {
        return path_;
    }

    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }

    const std::string& query() const {
        return query_;
    } 

    void setReceiveTime(Timestamp t) {
        receiveTime_ = t;
    }

    Timestamp receiveTime() const {
        return receiveTime_;
    }

    void addHeader(const char* start, const char* colon, const char* end) {
        std::string field(start, colon);
        ++colon;
        while(colon < end && std::isspace(static_cast<unsigned char>(*colon))) {
            ++colon;
        }
        std::string value(colon, end);
        auto lastNotSpace = value.find_last_not_of(" \t");
        if(lastNotSpace != std::string::npos) {
            value.resize(lastNotSpace + 1);
        } else {
            value.clear();
        }
        headers_[field] = value;
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        auto it = headers_.find(field);
        if(it != headers_.end()) {
            result = it->second;
        }
        return result;
    }

    const std::unordered_map<std::string, std::string>& headers() const {
        return headers_;
    }

    void swap(HttpRequest &that) { 
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receiveTime_.swap(that.receiveTime_);
        headers_.swap(that.headers_);
    }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    Timestamp receiveTime_;
    std::unordered_map<std::string, std::string> headers_;
};

