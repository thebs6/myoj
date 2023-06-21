#include "MysqlDataBase.h"
#include "ConnectionPool.h"
#include "model/user.hpp"

MysqlDataBase *MysqlDataBase::GetInstance() {
    static MysqlDataBase db;
    return &db;
}

Json MysqlDataBase::RegisterUser(Json &registerjson)
{
    Json resjson;
    try
    {
        std::string username = registerjson["username"].get<std::string>();
        std::string nickname = registerjson["nickname"].get<std::string>();
        std::string password = registerjson["password"].get<std::string>();
        std::string school = registerjson["school"].get<std::string>();
        std::string major = registerjson["major"].get<std::string>();
        
        // Check if the account already exists
        
        auto userModel = UserModel::getInstance();
        User user;
        bool accountExists = userModel->queryByUserName(username, user);
        if (accountExists)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "账户已存在，请重新填写！";
            return resjson;
        }

        bool nicknameExists = userModel->queryNickName(nickname);
        if (nicknameExists)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "昵称已存在，请重新填写！";
            return resjson;
        }

        User insertUser(0, username, nickname, password, school, major, 0, 0);
        bool insertSuccess = userModel->insert(insertUser);
        if (!insertSuccess)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库插入失败！";
            return resjson;
        }
        
        resjson["Result"] = "Success";
        resjson["Reason"] = "注册成功！";
        resjson["Id"] = insertUser.getId();
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户注册】数据库异常！";
        return resjson;
    }
}


Json MysqlDataBase::LoginUser(Json& loginjson) {
    Json resjson;
    try {
        std::string username = loginjson["username"].get<std::string>();
        std::string password = loginjson["password"].get<std::string>();

        auto userModel = UserModel::getInstance();
        User user;
        bool loginSuccess = userModel->queryByUserName(username, user);

        if (!loginSuccess || user.getPassword() != password) {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "账户或密码错误！";
            return resjson;
        }

        Json userInfo;
        userInfo["id"] = user.getId();
        userInfo["username"] = user.getUsername();
        userInfo["nickname"] = user.getNickname();
        userInfo["school"] = user.getSchool();
        userInfo["major"] = user.getMajor();
        userInfo["acnum"] = user.getAcnum();
        userInfo["submitnum"] = user.getSubmitnum();

        resjson["Result"] = "Success";
        resjson["Info"] = userInfo;
        return resjson;
    }
    catch (const std::exception& e) {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户登录】数据库异常！";
        return resjson;
    }
}

Json MysqlDataBase::LoginUserByToken(Json &loginjson) {
    Json resjson;
    try {
        string id = loginjson["id"].get<string>();

        auto userModel = UserModel::getInstance();
        User user;
        bool loginSuccess = userModel->queryByUserId(id, user);

        if (!loginSuccess) {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "不存在该用户";
            return resjson;
        }

        Json userInfo;
        userInfo["id"] = user.getId();
        userInfo["username"] = user.getUsername();
        userInfo["nickname"] = user.getNickname();
        userInfo["school"] = user.getSchool();
        userInfo["major"] = user.getMajor();
        userInfo["acnum"] = user.getAcnum();
        userInfo["submitnum"] = user.getSubmitnum();

        resjson["Result"] = "Success";
        resjson["Info"] = userInfo;
        return resjson;
    }
    catch (const std::exception& e) {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户登录】数据库异常！";
        return resjson;
    }
}