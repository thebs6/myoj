#pragma once 
#include "json.hpp"
#include "Logging.h"
#include "ConnectionPool.h"
#include "model/usermodel.hpp"
#include "model/statusRecordModel.hpp"
#include "model/problemListModel.hpp"
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

    Json LoginUserByToken(Json &loginjson);

    /*
        功能：更新用户的题目信息，当用户提交代码时
        传入：Json(UserId,ProblemId,Status)
        传出：bool(如果AC是否向其添加)
    */
    bool UpdateUserProblemInfo(Json &updatejson);

    /*
        功能：获取用户的Rank排名
        传入：Json(Page,PageSize)
        传出：Json(ArrayInfo[_id,Rank,Avatar,NickName,PersonalProfile,SubmitNum,ACNum],TotalNum)
    */
    Json SelectUserRank(Json &queryjson);

    /*
        功能：用户主页展示
        传入：Json(UserId)
        传出：Json(Result,Reason,_id,Avatar,NickName,PersonalProfile,School,Major,JoinTime,Solves,ACNum,SubmitNum)
    */
    Json SelectUserInfo(Json &queryjson);

    // ++++++++++++++++++++++++评测表StatusRecord+++++++++++++++++++++++++

    /*
        功能：插入待测评记录
        传入：Json(ProblemId,UserId,UserNickName,ProblemTitle,Language,Code)
        传出：SubmitId测评的ID
    */
    std::string InsertStatusRecord(Json &insertjson);
    /*
        功能：更新测评记录
        传入：Json(SubmitId,Status,RunTime,RunMemory,Length,ComplierInfo,
        TestInfo[(Status,StandardInput,StandardOutput,PersonalOutput,RunTime,RunMemory)])
        传出：bool
    */
    bool UpdateStatusRecord(Json &updatejson);

    /*
        功能：更新题目的状态数量
        传入：Json(ProblemId,Status)
        传出：bool
    */
    bool UpdateProblemStatusNum(Json &updatejson);

private:
    /*
        功能：获取某一个集合中最大的ID
    */
    MysqlDataBase(){};

    ~MysqlDataBase(){};

private:
};