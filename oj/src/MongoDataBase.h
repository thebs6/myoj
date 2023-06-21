#ifndef MONGODATABASE_H
#define MONGODATABASE_H

#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include "json.hpp"
#include "Logging.h"
using Json = nlohmann::json;
#include <atomic>

class MoDB
{
public:
    // 局部静态特性的方式实现单实例
    static MoDB *GetInstance();

    // +++++++++++++++++++++++用户表User+++++++++++++++++++++++++++++
    /*
        功能：用户注册
        传入：Json(NickName,Account,PassWord,PersonalProfile,School,Major)
        传出：Json(Result,Reason)
    */
    Json RegisterUser(Json &registerjson);

    /*
        功能：用户登录
        传入：Json(Account,PassWord)
        传出：Json(Result,Reason,Info(_id,NickName,Avatar,CommentLikes,Solves,Authority))
    */
    Json LoginUser(Json &loginjson);

    /*
        功能：用户登录通过Token
        传入：Json(UserId)
        传出：Json(Result,Reason,Info(_id,NickName,Avatar,CommentLikes,Solves,Authority))
    */
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

    /*
        功能：更改用户信息
        传入：Json(UserId,Avatar,PersonalProfile,School,Major)
        传出：Json(Result,Reason)
    */
    Json UpdateUserInfo(Json &updatejson);

    /*
        功能：查询用户表，用于修改用户
        传入：Json(UserId)
        传出：Json(_id,Avatar,NickName,PersonalProfile,School,Major)
    */
    Json SelectUserUpdateInfo(Json &queryjson);

    /*
        功能：分页查询用户列表
        传入：Json(Page,PageSize)
        传出：Json(_id,NickName,PersonalProfile,School,Major,JoinTime,TotalNum)
    */
    Json SelectUserSetInfo(Json &queryjson);

    /*
        功能：删除用户
        传入：Json(UserId)
        传出：Json(Result,Reason)
    */
    Json DeleteUser(Json &deletejson);

    /*
        功能：查询所有用户的权限
        传入：void
        传出：Json(Result,_id,Authority)
    */
    Json SelectUserAuthority();
    // ++++++++++++++++++++++++++题目表Problem+++++++++++++++++++++++++++++

    /*
        功能：管理员查询题目信息
        传入：Json(ProblemId)
        传出：Json(Result,Reason,_id,Title,Description,TimeLimit,MemoryLimit,UserNickName,JudgeNum,Tags)
    */
    Json SelectProblemInfoByAdmin(Json &queryjson);

    /*
        功能：获取题目信息
        传入：Json(ProblemId)
        传出：Json(Result,Reason,_id,Title,Description,TimeLimit,MemoryLimit,JudgeNum,SubmitNum,ACNum,UserNickName,Tags)
    */
    Json SelectProblem(Json &queryjson);
    /*
        功能：插入题目
        传入：Json(Title,Description,TimeLimit,MemoryLimit,JudgeNum,Tags,UseNickName)
        传出：Json(Reuslt,Reason,ProblemId)
    */
    Json InsertProblem(Json &insertjson);

    /*
        功能：修改题目信息
        传入：Json(ProblemId,Title,Description,TimeLimit,MemoryLimit,JudgeNum,Tags,UseNickName)
        传出：Json(Result,Reason)
    */
    Json UpdateProblem(Json &updatejson);

    /*
        功能：删除题目
        传入：Json(ProblemId)
        传出：Json(Result,Reason)
    */
    Json DeleteProblem(Json &deletejson);

    /*
        功能：分页获取题目列表（包含查询条件，暂时未添加）
        前端传入
        Json(SearchInfo,Page,PageSize,MatchString)
        后端传出
        Json(ProblemId,Title,SubmitNum,CENum,ACNum,WANum,TLENum,MLENum,SENum,Tags),TotalNum
    */
    Json SelectProblemList(Json &queryjson);

    /*
        功能：管理员分页获取题目列表
        传入：Json(Page,PageSize)
        传出：Json(ArrayInfo([ProblemId,Title,SubmitNum,CENum,ACNum,WANum,TLENum,MLENum,SENum,Tags]),TotalNum)
    */
    Json SelectProblemListByAdmin(Json &queryjson);

    /*
        功能：更新题目的状态数量
        传入：Json(ProblemId,Status)
        传出：bool
    */
    bool UpdateProblemStatusNum(Json &updatejson);

    /*
        功能：获取题目的所有标签
        传入：void
        传出：Json(tags)
    */
    Json getProblemTags();
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
        功能：分页查询测评记录
        传入：Json(SearchInfo,PageSize,Page)
        传出：测评全部信息，详情请见MongoDB集合表
    */
    Json SelectStatusRecordList(Json &queryjson);

    /*
        功能：查询测评记录
        传入：Json(SubmitId)
        传出：全部记录，详情请看MongoDB集合表
    */
    Json SelectStatusRecord(Json &queryjson);

    // +++++++++++++++++++++++++Disscuss 讨论表++++++++++++++++++++++++++++

    /*
        功能：添加讨论
        传入：Json(Title,Content,ParentId,UserId) 如果是父讨论ParentId=0
        传出：Json(Result)
    */
    Json InsertDiscuss(Json &insertjson);

    /*
        功能：分页查询讨论
        传入：Json(SearchInfo,Page,PageSize)
        传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
    */
    Json SelectDiscussList(Json &queryjson);

    /*
        功能：管理员分页查询讨论
        传入：Json(Page,PageSize)
        传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
    */
    Json SelectDiscussListByAdmin(Json &queryjson);

    /*
        功能：查询讨论的详细信息，主要是编辑时的查询
        传入：Json(DiscussId)
        传出：Json(Result,Reason,Title,Content)
    */
    Json SelectDiscussByEdit(Json &queryjson);

    /*
        功能：查询讨论的详细内容，并且将其浏览量加一
        传入：Json(DiscussId)
        传出：Json(Resutl,Reason,Content,Views,Comments,CreateTime,UpdateTime,User.NickName,User.Avatar)
    */
    Json SelectDiscuss(Json &queryjson);

    /*
        功能：修改讨论的评论数
        传入：Json(DiscussId,Num)
        传出：bool
    */
    bool UpdateDiscussComments(Json &updatejson);

    /*
        功能：更新讨论
        传入：Json(DiscussId,Title,Content)
        传出；Json(Result,Reason)
    */
    Json UpdateDiscuss(Json &updatejson);

    /*
        功能：删除讨论
        传入：Json(DiscussId)
        传出：Json(Result,Reason)
    */
    Json DeleteDiscuss(Json &deletejson);

    // +++++++++++++++++++++++++Solution 题解表++++++++++++++++++++++++++++

    /*
        功能：添加题解
        传入：Json(Title,Content,ParentId,UserId,Public)
        传出：Json(Result,Reason)
    */
    Json InsertSolution(Json &insertjson);

    /*
        功能：分页查询题解（公开题解）
        传入：Json(SearchInfo,Page,PageSize)
        传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
    */
    Json SelectSolutionList(Json &queryjson);

    /*
        功能：管理员分页查询题解
        传入：Json(Page,PageSize)
        传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
    */
    Json SelectSolutionListByAdmin(Json &queryjson);

    /*
        功能：查询题解的详细信息，主要是编辑时的查询
        传入：Json(SolutionId)
        传出：Json(Result,Reason,Title,Content,Public)
    */
    Json SelectSolutionByEdit(Json &queryjson);

    /*
        功能：查询题解的详细内容，并且将其浏览量加一
        传入：Json(SolutionId)
        传出：Json(Result,Reason,Title,Content,Views,Comments,CreateTime,UpdateTime,User.NicaName,User.Avatar)
    */
    Json SelectSolution(Json &queryjson);

    /*
        功能：修改题解的评论数
        传入：Json(ArticleId,Num)
        传出：bool
    */
    bool UpdateSolutionComments(Json &updatejson);

    /*
        功能：更新题解
        传入：Json(SolutionId,Title,Content,Public)
        传出；Json(Result,Reason)
    */
    Json UpdateSolution(Json &updatejson);

    /*
        功能：删除题解
        传入：Json(ArticleId)
        传出：Json(Result,Reason)
    */
    Json DeleteSolution(Json &deletejson);

    // +++++++++++++++++++++++++Announcement 公告表++++++++++++++++++++++++++++

    /*
        功能：添加公告
        传入：Json(Title,Content,UserId,Level)
        传出：Json(Result,Reason)
    */
    Json InsertAnnouncement(Json &insertjson);

    /*
        功能：分页查询公告
        传入：Json(Page,PageSize)
        传出：Json([Result,Reason,_id,Title,Views,Comments,CreateTime],TotalNum)
    */
    Json SelectAnnouncementList(Json &queryjson);

    /*
        功能：查询公告的详细信息，主要是编辑时的查询
        传入：Json(AnnouncementId)
        传出：Json(Result,Reason,Title,Content,Level)
    */
    Json SelectAnnouncementByEdit(Json &queryjson);

    /*
        功能：查询公告的详细内容，并且将其浏览量加一
        传入：Json(AnnouncementId)
        传出：Json(Title,Content,Views,Comments,CreateTime,UpdateTime)
    */
    Json SelectAnnouncement(Json &queryjson);

    /*
        功能：修改公告的评论数
        传入：Json(ArticleId,Num)
        传出：bool
    */
    bool UpdateAnnouncementComments(Json &updatejson);

    /*
        功能：更新公告
        传入：Json(AnnouncementId,Title,Content,Level)
        传出；Json(Result,Reason)
    */
    Json UpdateAnnouncement(Json &updatejson);

    /*
        功能：删除公告
        传入：Json(AnnouncementId)
        传出：Json(Result,Reason)
    */
    Json DeleteAnnouncement(Json &deletejson);

    // ++++++++++++++++++++++评论表 Comment+++++++++++++++++++++++

    /*
        功能：管理员查询评论
        传入：Json(Page,PageSize)
        传出：Json(_id,ParentId,ParentType,Content,CreateTime,
            Child_Comments._id,Child_Comments.Content,Child_Comments.CreateTime)
    */
    Json SelectCommentListByAdmin(Json &queryjson);
    /*
        功能：查询父评论
        传入：Json(ParentId,Skip,Limie,SonNum)
        传出：
        Json(ParentId,Content,Likes,CreateTime,Child_Total,
        User(Avatar,NickName),
        Child_Comments(_id,Content,Likes,CreateTime,User(Avatar,NickName)))
    */
    Json getFatherComment(Json &queryjson);

    /*
        功能：获取子评论
        传入：Json(ParentId,Skip,Limit)
        传出：Json(Child_Total,Child_Comments(_id,Content,Likes,CreateTime,User(NickName,Avatar)))
    */
    Json getSonComment(Json &queryjson);

    /*
        功能：插入父评论
        传入：Json(ParentId,Content,UserId)
        传出：Json(_id,CreateTime)
    */
    Json InsertFatherComment(Json &insertjson);

    /*
        功能：插入子评论
        传入：Json(ParentId,Content,UserId)
        传出：Json(_id,CreateTime)
    */
    Json InsertSonComment(Json &insertjson);

    /*
        功能：删除某一篇文章（讨论，题解，公告）的所有文章，主要服务于删除文章
        传入：Json(ArticleId)
        传出：bool
    */
    bool DeleteArticleComment(Json &deletejson);

    /*
        功能：删除父评论
        传入：Json(CommentId)
        传出：Json(Result,Reason,DeleteNum)
    */
    Json DeleteFatherComment(Json &deletejson);

    /*
        功能：删除子评论
        传入：Json(CommentId)
        传出：Json(Result,Reason,DeleteNum)
    */
    Json DeleteSonComment(Json &deletejson);

private:
    /*
        功能：获取某一个集合中最大的ID
    */
    int64_t GetMaxId(std::string name);
    MoDB();

    ~MoDB();

private:
    mongocxx::instance instance{};                                                            // This should be done only once.
    mongocxx::uri uri{"mongodb://root:root@localhost:27017/?authMechanism=SCRAM-SHA-1"}; // 连接配置
    mongocxx::pool pool{uri};                                                                 // 连接池

    std::atomic_int64_t m_problemid;      // 题目ID
    std::atomic_int64_t m_statusrecordid; // 测评ID
    std::atomic_int64_t m_commentid;      // 评论ID
    std::atomic_int64_t m_articleid;      // 文章ID
};

#endif