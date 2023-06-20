#pragma once 
#include "json.hpp"
#include "Logging.h"
#include "ConnectionPool.h"
#include "model/usermodel.hpp"
using Json = nlohmann::json;
using string = std::string;
#include <atomic>

class MysqlDataBase
{
public:
    // 局部静态特性的方式实现单实例
    static MysqlDataBase *GetInstance();

    // +++++++++++++++++++++++用户表User+++++++++++++++++++++++++++++
    /*
        功能：用户注册
        传入：Json(NickName,Account,PassWord,PersonalProfile,School,Major)
        传出：Json(Result,Reason)
    */
    Json RegisterUser(Json &registerjson);
    Json LoginUser(Json &loginjson);

private:
    /*
        功能：获取某一个集合中最大的ID
    */
    MysqlDataBase(){};

    ~MysqlDataBase(){};

private:
};