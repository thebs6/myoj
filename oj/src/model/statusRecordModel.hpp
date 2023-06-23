#include "user.hpp"
#include "ConnectionPool.h"
#include "Connector.h"
#include "Logging.h"
#include "statusRecord.hpp"
#include "json.hpp"

using Json = nlohmann::json;

class statusRecordModel {
public:
    static statusRecordModel* getInstance() {
        static statusRecordModel ins;
        return &ins;
    }

    // auto insert(StatusRecord& statusRecord) -> bool {
    //     char sql[1024] = {0};
    //     sprintf(sql, "insert into status_record(problem_id, username, user_nickname, problem_title, status, run_time, run_memory, length, language, submit_time, code, compiler_info) values('%lld', '%s', '%s', '%s', '%s', '%d', '%d')",
    //         statusRecord.getProblem(), statusRecord.getNickname().c_str(), statusRecord.getProblem_title().c_str(), statusRecord.getStatus(), statusRecord.getRun_time().c_str(), statusRecord.getRun_memory(), statusRecord.getLength(),
    //         statusRecord.getLanguage().c_str(), statusRecord.getSubmit_time().c_str(), statusRecord.getCode().c_str(), statusRecord.getCodecompiler_info().c_str());
    //     auto conn = ConnectionPool::getConnectionPool()->getConnection();
    //     uint64_t id;
    //     if(conn->insert(sql, id)) {
    //         statusRecord.setId(id);
    //         return true;
    //     }
    //     return false;
    // }
    
    auto insert(Json& insertjson) -> bool { 
        char sql[1024] = {0};
        sprintf(sql,  "INSERT INTO status_record (problem_id, username, user_nickname, problem_title, status, run_time, run_memory, length, language, submit_time, code, compiler_info) VALUES ('%ld', '%ld', '%s', '%s', '%d', '%s', '%s', '%s', '%s', '%s', '%s', '%s');",
            insertjson["ProblemId"].get<long>(), insertjson["UserId"].get<long>(), insertjson["UserNickName"].get<std::string>().c_str(), insertjson["ProblemTitle"].get<std::string>().c_str(),
            insertjson["Status"].get<int>(), insertjson["RunTime"].get<std::string>().c_str(), insertjson["RunMemory"].get<std::string>().c_str(), insertjson["Length"].get<std::string>().c_str(),
            insertjson["Language"].get<std::string>().c_str(), insertjson["SubmitTime"].get<std::string>().c_str(), insertjson["Code"].get<std::string>().c_str(), insertjson["CompilerInfo"].get<std::string>().c_str());
        std::cout << sql << std::endl;
        auto conn = ConnectionPool::getConnectionPool()->getConnection();
        uint64_t id;
        if(conn->insert(sql, id)) {
            insertjson["id"] = id;
            return true;
        }
        return false;
    }

private:
    statusRecordModel(){};
    ~statusRecordModel(){};
};