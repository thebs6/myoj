#include "user.hpp"
#include "ConnectionPool.h"
#include "Connector.h"
#include "Logging.h"
class UserModel {
public:
    static UserModel* getInstance() {
        static UserModel ins;
        return &ins;
    }

    auto insert(User& user) -> bool {
        char sql[1024] = {0};
        sprintf(sql, "insert into user(username, nickname, password, school, major, acnum, submitnum) values('%s', '%s', '%s', '%s', '%s', '%d', '%d')",
            user.getUsername().c_str(), user.getNickname().c_str(), user.getPassword().c_str(), user.getSchool().c_str(), user.getMajor().c_str(), 0, 0);
        auto conn = ConnectionPool::getConnectionPool()->getConnection();
        uint64_t id;
        if(conn->insert(sql, id)) {
            user.setId(id);
            return true;
        }
        return false;
    }

    auto queryByUserName(std::string username, User& user) -> bool {
        std::string sql = "SELECT * FROM user WHERE username = '" + username + "'";
        auto conn = ConnectionPool::getConnectionPool()->getConnection();

        if (conn->query(sql) && conn->next()) {
            user.setId(std::stol(conn->value(0)));
            user.setUsername(conn->value(1));
            user.setNickname(conn->value(2));
            user.setPassword(conn->value(3));
            user.setSchool(conn->value(4));
            user.setMajor(conn->value(5));
            user.setAcnum(std::stoi(conn->value(6)));
            user.setSubmitnum(std::stoi(conn->value(7)));

            return true;
        }

        return false;
    }

    auto queryByUserId(string userid, User& user) -> bool {
        std::string sql = "SELECT * FROM user WHERE id = " + userid;
        LOG_DEBUG << sql ;
        auto conn = ConnectionPool::getConnectionPool()->getConnection();

        if (conn->query(sql) && conn->next()) {
            user.setId(std::stol(conn->value(0)));
            user.setUsername(conn->value(1));
            user.setNickname(conn->value(2));
            user.setPassword(conn->value(3));
            user.setSchool(conn->value(4));
            user.setMajor(conn->value(5));
            user.setAcnum(std::stoi(conn->value(6)));
            user.setSubmitnum(std::stoi(conn->value(7)));

            return true;
        }

        return false;
    }

    auto queryNickName(std::string nickname) -> bool {
        std::string sql = "SELECT nickname FROM user WHERE nickname = '" + nickname + "'";
        auto conn = ConnectionPool::getConnectionPool()->getConnection();

        if (conn->query(sql) && conn->next()) {
            return true;
        }
        return false;
    }

    bool verifyCredentials(const std::string& username, const std::string& password) {
        std::string sql = "SELECT username FROM user WHERE username = '" + username + "' AND password = '" + password + "'";
        auto conn = ConnectionPool::getConnectionPool()->getConnection();

        if (conn->query(sql) && conn->next()) {
            return true;
        }
        return false;
    }

    bool updateProblemStatus(const std::string& username, bool ac, int submitNumAdd) {
        char sql[1024] = {0};
        string acUpdate = ac ? ", ac_num=ac_num+1" : "";
        sprintf(sql, "update user set submit_num=submit_num+%d %s where username=%s", submitNumAdd, acUpdate.c_str(), username.c_str());
        auto conn = ConnectionPool::getConnectionPool()->getConnection();

        if (conn->update(sql)) {
            return true;
        }
        return false;
    }

private:
    UserModel(){};
    ~UserModel(){};
};