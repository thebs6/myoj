#include "UserList.h"
#include "MongoDataBase.h"
#include "MysqlDataBase.h"
#include "./utils/snowflake.hpp"
#include "RedisDataBase.h"
#include <iostream>
using namespace std;

// 雪花算法
using snowflake_t = snowflake<1534832906275L, std::mutex>;
snowflake_t uuid_token;

UserList *UserList::GetInstance()
{
    static UserList userlist;
    return &userlist;
}
Json UserList::RegisterUser(Json &registerjson)
{
    Json json = MysqlDataBase::GetInstance()->RegisterUser(registerjson);
    // 将其权限加入用户权限表中
    if (json["Result"].get<std::string>() == "Success")
    {
        int64_t id = json["Id"].get<int64_t>();
        UserAuthorityMap[id] = 3;
    }
    return json;
}

Json UserList::LoginUser(Json &loginjson)
{
    Json json = MysqlDataBase::GetInstance()->LoginUser(loginjson);

    if (json["Result"] == "Success")
    {
        uuid_token.init(1, 1);
        // 获取Token
        int64_t token = uuid_token.nextid();

        // Redis存入Token
        ReDB::GetInstance()->SetToken(to_string(token), to_string(json["Info"]["id"].get<uint64_t>()));

        json["Info"]["Token"] = to_string(token);
    }

    return json;
}

Json UserList::LoginUserByToken(Json &loginjson)
{
    string token = loginjson["Token"].get<std::string>();

    // 根据Token查询UserId
    string userid = ReDB::GetInstance()->GetUserIdByToken(token);

    loginjson["UserId"] = userid;

    return MoDB::GetInstance()->LoginUserByToken(loginjson);
}

bool UserList::UpdateUserProblemInfo(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateUserProblemInfo(updatejson);
}

Json UserList::SelectUserRank(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserRank(queryjson);
}

Json UserList::SelectUserInfo(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserInfo(queryjson);
}

Json UserList::UpdateUserInfo(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateUserInfo(updatejson);
}

Json UserList::SelectUserUpdateInfo(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserUpdateInfo(queryjson);
}

Json UserList::SelectUserSetInfo(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserSetInfo(queryjson);
}

Json UserList::DeleteUser(Json &deletejson)
{
    Json json = MoDB::GetInstance()->DeleteUser(deletejson);
    if (json["Result"].get<std::string>() == "Success")
    {
        int64_t id = stoll(deletejson["UserId"].get<std::string>());
        UserAuthorityMap.erase(id);
    }
    return json;
}

bool UserList::InitUserAuthority()
{
    Json json = MoDB::GetInstance()->SelectUserAuthority();
    if (json["Result"] == "Fail")
        return false;

    for (auto info : json["ArrayInfo"])
    {
        UserAuthorityMap[info["_id"].get<int64_t>()] = info["Authority"].get<int>();
    }

    return true;
}

// 将json 的Token 转化为 VerifyId
void TokenToVerifyId(Json &json)
{
    if (!json["Token"].empty())
    {
        json["VerifyId"] = ReDB::GetInstance()->GetUserIdByToken(json["Token"].get<std::string>());
    }
}

int UserList::GetUserAuthority(Json &json)
{
    /*
        用户权限
        1：游客
        3：普通用户
        5：管理员
    */
    // 如果未发现ID
    if (json["VerifyId"].empty())
        return 1;
    try
    {
        int64_t id = stoll(json["VerifyId"].get<std::string>());
        // 如果未查询到该用户ID或者用户ID为0是游客
        if (UserAuthorityMap[id] == 0)
            return 1;
        return UserAuthorityMap[id];
    }
    catch (const std::exception &e)
    {
        return 1;
    }
}
// 是否是普通用户或以上
bool UserList::IsOrdinaryUser(Json &json)
{
    TokenToVerifyId(json);
    if (GetUserAuthority(json) >= 3)
        return true;
    else
        return false;
}

// 是否是作者本人或以上
bool UserList::IsAuthor(Json &json)
{
    TokenToVerifyId(json);
    int authority = GetUserAuthority(json);

    if (authority < 3)
        return false;

    if (authority >= 5)
        return true;
    try
    {
        int64_t verifyid = stoll(json["VerifyId"].get<std::string>());
        int64_t userid = stoll(json["UserId"].get<std::string>());

        if (verifyid == userid)
            return true;
        else
            return false;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

// 是否是管理员或以上
bool UserList::IsAdministrator(Json &json)
{
    TokenToVerifyId(json);
    if (GetUserAuthority(json) >= 5)
        return true;
    else
        return false;
}

UserList::UserList()
{
}
UserList::~UserList()
{
}