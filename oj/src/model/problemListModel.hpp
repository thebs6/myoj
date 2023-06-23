#pragma once
#include "user.hpp"
#include "ConnectionPool.h"
#include "Connector.h"
#include "Logging.h"
#include "json.hpp"
#include <cstdio>

using Json = nlohmann::json;

class ProblemListModel {
public:
    static ProblemListModel* getInstance() {
        static ProblemListModel ins;
        return &ins;
    }
    
    auto update(uint64_t id, std::string addCol) -> bool { 
        char sql[1024] = {0};
        sprintf(sql, "UPDATE problem SET submit_num = submit_num + 1, %s = %s + 1 WHERE id = %lu",
        addCol.c_str(), addCol.c_str(), id);
        if(ConnectionPool::getConnectionPool()->getConnection()->update(sql)) {
            return true;;
        }
        return false;
    }

private:
    ProblemListModel(){};
    ~ProblemListModel(){};
};