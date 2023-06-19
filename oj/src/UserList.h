#ifndef USERLIST_H
#define USERLIST_H
#include "json.hpp"
using Json = nlohmann::json;
#include <unordered_map>

class UserList
{
public:
    // 局部静态特性的方式实现单实例
    static UserList *GetInstance();
    // 注册用户
    Json RegisterUser(Json &registerjson);

    // 登录用户
    Json LoginUser(Json &loginjson);

    // 登录用户通过Token
    Json LoginUserByToken(Json &loginjson);

    // 更新用户题目信息
    bool UpdateUserProblemInfo(Json &updatejson);

    // 获取用户Rank排名
    Json SelectUserRank(Json &queryjson);

    // 获取用户大部分信息
    Json SelectUserInfo(Json &queryjson);

    // 更新用户信息
    Json UpdateUserInfo(Json &updatejson);

    // 获取用户信息以供修改
    Json SelectUserUpdateInfo(Json &queryjson);

    /*
        功能：分页查询用户列表
        传入：Json(Page,PageSize)
        传出：Json(_id,NickName,PersonalProfile,School,Major,JoinTime,Authority)
    */
    Json SelectUserSetInfo(Json &queryjson);

    // 删除用户
    Json DeleteUser(Json &deletejson);

    // 初始化用户权限
    bool InitUserAuthority();

    // 获取用户权限
    int GetUserAuthority(Json &json);

    // 权限是否是普通用户或以上
    bool IsOrdinaryUser(Json &json);

    // 是否是作者本人或以上
    bool IsAuthor(Json &json);

    // 是否是管理员或以上
    bool IsAdministrator(Json &json);

private:
    UserList();
    ~UserList();

private:
    // 用户权限的哈希表，键：用户ID，值：用户权限
    std::unordered_map<int64_t, int> UserAuthorityMap;
};
#endif