#include "UserService.h"

#include "snowflake.hpp"
// #include "RedisDataBase.h"
#include <iostream>
#include <mutex>
using namespace std;

// 雪花算法
using snowflake_t = snowflake<1534832906275L, std::mutex>;
snowflake_t uuid_token;


Json UserService::RegisterUser(Json &registerjson)
{
    
    // 将其权限加入用户权限表中
    if (json["Result"].asString() == "Success")
    {
        int64_t id = json["_id"].asInt64();
        UserAuthorityMap[id] = 3;
    }
    return json;
}

Json UserService::LoginUser(Json &loginjson)
{
    Json json = MoDB::GetInstance()->LoginUser(loginjson);

    if (json["Result"] == "Success")
    {
        uuid_token.init(1, 1);
        // 获取Token
        int64_t token = uuid_token.nextid();

        // Redis存入Token
        ReDB::GetInstance()->SetToken(to_string(token), json["Info"]["_id"].asString());

        json["Info"]["Token"] = to_string(token);
    }

    return json;
}

Json UserService::LoginUserByToken(Json &loginjson)
{
    string token = loginjson["Token"].asString();

    // 根据Token查询UserId
    string userid = ReDB::GetInstance()->GetUserIdByToken(token);

    loginjson["UserId"] = userid;

    return MoDB::GetInstance()->LoginUserByToken(loginjson);
}

bool UserService::UpdateUserProblemInfo(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateUserProblemInfo(updatejson);
}

Json UserService::SelectUserRank(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserRank(queryjson);
}

Json UserService::SelectUserInfo(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserInfo(queryjson);
}

Json UserService::UpdateUserInfo(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateUserInfo(updatejson);
}

Json UserService::SelectUserUpdateInfo(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserUpdateInfo(queryjson);
}

Json UserService::SelectUserSetInfo(Json &queryjson)
{
    return MoDB::GetInstance()->SelectUserSetInfo(queryjson);
}

Json UserService::DeleteUser(Json &deletejson)
{
    Json json = MoDB::GetInstance()->DeleteUser(deletejson);
    if (json["Result"].asString() == "Success")
    {
        int64_t id = stoll(deletejson["UserId"].asString());
        UserAuthorityMap.erase(id);
    }
    return json;
}

bool UserService::InitUserAuthority()
{
    Json json = MoDB::GetInstance()->SelectUserAuthority();
    if (json["Result"] == "Fail")
        return false;

    for (auto info : json["ArrayInfo"])
    {
        UserAuthorityMap[info["_id"].asInt64()] = info["Authority"].asInt();
    }

    return true;
}

// 将json 的Token 转化为 VerifyId
void TokenToVerifyId(Json &json)
{
    if (!json["Token"].isNull())
    {
        json["VerifyId"] = ReDB::GetInstance()->GetUserIdByToken(json["Token"].asString());
    }
}

int UserService::GetUserAuthority(Json &json)
{
    /*
        用户权限
        1：游客
        3：普通用户
        5：管理员
    */
    // 如果未发现ID
    if (json["VerifyId"].isNull())
        return 1;
    try
    {
        int64_t id = stoll(json["VerifyId"].asString());
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
bool UserService::IsOrdinaryUser(Json &json)
{
    TokenToVerifyId(json);
    if (GetUserAuthority(json) >= 3)
        return true;
    else
        return false;
}

// 是否是作者本人或以上
bool UserService::IsAuthor(Json &json)
{
    TokenToVerifyId(json);
    int authority = GetUserAuthority(json);

    if (authority < 3)
        return false;

    if (authority >= 5)
        return true;
    try
    {
        int64_t verifyid = stoll(json["VerifyId"].asString());
        int64_t userid = stoll(json["UserId"].asString());

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
bool UserService::IsAdministrator(Json &json)
{
    TokenToVerifyId(json);
    if (GetUserAuthority(json) >= 5)
        return true;
    else
        return false;
}

UserService::UserList()
{
}
UserService::~UserList()
{
}