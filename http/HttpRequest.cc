#include "HttpRequest.h"


const std::unordered_map<std::string, Method> HttpRequest::kStringToHttpMethod = {
            {"GET", kGet},
            {"POST", kPost},
            {"HEAD", kHead},
            {"PUT", kPut},
            {"DELETE", kDelete},
};