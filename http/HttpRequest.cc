#include "HttpRequest.h"


const std::unordered_map<std::string, HttpRequest::Method> HttpRequest::kStringToHttpMethod = {
            {"GET", kGet},
            {"POST", kPost},
            {"HEAD", kHead},
            {"PUT", kPut},
            {"DELETE", kDelete},
};