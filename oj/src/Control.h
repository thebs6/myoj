#ifndef CONTROL_H
#define CONTROL_H
#include <string>
#include "json.hpp"
using Json = nlohmann::json;
class Control
{
public:
    // ----------------用户表 User-----------------
    // 注册用户
    Json RegisterUser(Json &registerjson);

    // 登录用户
    Json LoginUser(Json &loginjson);

    // 登录用户通过Token
    Json LoginUserByToken(Json &loginjson);

    // 获取用户Rank排名
    Json SelectUserRank(Json &queryjson);

    // 获取用户大部分信息
    Json SelectUserInfo(Json &queryjson);

    // 更新用户信息
    Json UpdateUserInfo(Json &updatejson);

    // 获取用户信息，以供修改
    Json SelectUserUpdateInfo(Json &queryjson);

    // 分页获取用户信息
    Json SelectUserSetInfo(Json &queryjson);

    // 删除用户
    Json DeleteUser(Json &deletejson);
    // ---------------题目 Problem -------------------

    // 管理员查看题目数据
    Json SelectProblemInfoByAdmin(Json &queryjson);

    // 用户查询题目信息
    Json SelectProblem(Json &queryjson);

    // 编辑题目
    Json EditProblem(Json &insertjson);

    // 删除题目
    Json DeleteProblem(Json &deletejson);

    // 返回题库
    Json SelectProblemList(Json &queryjson);

    // 管理员查询列表
    Json SelectProblemListByAdmin(Json &queryjson);
    // 返回判题信息
    Json GetJudgeCode(Json judgejson);

    // ---------------测评表-------------------------
    // 返回状态记录的信息
    Json SelectStatusRecordList(Json &queryjson);

    // 查询一条详细测评记录
    Json SelectStatusRecord(Json &queryjson);

    // ---------------评论Comment-------------------

    // 管理员查询评论
    Json SelectCommentListByAdmin(Json &queryjson);

    // 获取评论 根据Id
    Json GetComment(Json &queryjson);

    // 插入评论
    Json InsertComment(Json &insertjson);

    // 删除评论
    Json DeleteComment(Json &deletejson);

    // ----------------------公告---------------------------
    // 查询公告列表
    Json SelectAnnouncementList(Json &queryjson);

    // 管理员查询公告列表
    Json SelectAnnouncementListByAdmin(Json &queryjson);

    // 查询公告
    Json SelectAnnouncement(Json &queryjson);

    // 查询公告 进行编辑
    Json SelectAnnouncementByEdit(Json &queryjson);

    // 插入公告
    Json InsertAnnouncement(Json &insertjson);

    // 更新公告
    Json UpdateAnnouncement(Json &updatejson);

    // 删除公告
    Json DeleteAnnouncement(Json &deletejson);

    // ----------------------题解-----------------------
    // 查询题解列表
    Json SelectSolutionList(Json &queryjson);

    // 管理员查询
    Json SelectSolutionListByAdmin(Json &queryjson);

    // 查询题解
    Json SelectSolution(Json &queryjson);

    // 查询题解进行编辑
    Json SelectSolutionByEdit(Json &queryjson);

    // 插入题解
    Json InsertSolution(Json &insertjson);

    // 更新题解
    Json UpdateSolution(Json &updatejson);

    // 删除题解
    Json DeleteSolution(Json &deletejson);

    // ----------------------讨论-----------------------
    // 查询题解列表
    Json SelectDiscussList(Json &queryjson);

    // 管理员查询
    Json SelectDiscussListByAdmin(Json &queryjson);

    // 查询题解
    Json SelectDiscuss(Json &queryjson);

    // 查询题解进行编辑
    Json SelectDiscussByEdit(Json &queryjson);

    // 插入题解
    Json InsertDiscuss(Json &insertjson);

    // 更新题解
    Json UpdateDiscuss(Json &updatejson);

    // 删除题解
    Json DeleteDiscuss(Json &deletejson);

    Json GetTags(Json &queryjson);
    Control();

    ~Control();

private:
};

#endif