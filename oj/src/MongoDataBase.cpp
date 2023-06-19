#include "MongoDataBase.h"
#include <cstdint>
#include <iostream>
#include "./utils/snowflake.hpp"

#include <ctime>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/pipeline.hpp>

#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

using namespace std;
// 雪花算法
using snowflake_t = snowflake<1534832906275L, std::mutex>;
snowflake_t uuid;

// 获取时间
string GetTime()
{
    time_t t = time(nullptr);
    struct tm *now = localtime(&t);
    char str[50];
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", now);
    return (string)str;
}

MoDB *MoDB::GetInstance()
{
    static MoDB modb;
    return &modb;
}

/*
    功能：用户注册
    传入：Json(NickName,Account,PassWord,PersonalProfile,School,Major)
    传出：Json(Result,Reason)
*/
Json MoDB::RegisterUser(Json &registerjson)
{
    Json resjson;
    try
    {
        string account = registerjson["Account"].get<std::string>();
        string nickname = registerjson["NickName"].get<std::string>();
        string password = registerjson["PassWord"].get<std::string>();
        string personalprofile = registerjson["PersonalProfile"].get<std::string>();
        string school = registerjson["School"].get<std::string>();
        string major = registerjson["Major"].get<std::string>();
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        mongocxx::cursor cursor = usercoll.find({make_document(kvp("Account", account.data()))});
        // 判断账户是否存在
        if (cursor.begin() != cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "账户已存在，请重新填写！";
            return resjson;
        }

        // 检查昵称是否存在
        cursor = usercoll.find({make_document(kvp("NickName", nickname.data()))});
        if (cursor.begin() != cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "昵称已存在，请重新填写！";
            return resjson;
        }

        uuid.init(1, 1);
        int64_t id = uuid.nextid();
        string jointime = GetTime();
        // 默认头像
        string avatar = "http://175.178.54.194:8081/image/1";
        // 插入
        bsoncxx::builder::stream::document document{};
        document
            << "_id" << id
            << "Avatar" << avatar.data()
            << "NickName" << nickname.data()
            << "Account" << account.data()
            << "PassWord" << password.data()
            << "PersonalProfile" << personalprofile.data()
            << "School" << school.data()
            << "Major" << major.data()
            << "JoinTime" << jointime.data()
            << "CommentLikes" << open_array << close_array
            << "Solves" << open_array << close_array
            << "SubmitNum" << 0
            << "ACNum" << 0
            << "Authority" << 3;

        auto result = usercoll.insert_one(document.view());
        if ((*result).result().inserted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库插入失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        resjson["Reason"] = "注册成功！";
        resjson["_id"] = id;
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户注册】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：用户登录
    传入：Json(Account,PassWord)
    传出：Json(Result,Reason,Info(_id,NickName,Avatar,CommentLikes,Solves,Authority))
*/
Json MoDB::LoginUser(Json &loginjson)
{
    Json resjson;
    try
    {
        string account = loginjson["Account"].get<std::string>();
        string password = loginjson["PassWord"].get<std::string>();
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};
        document
            << "Account" << account.data()
            << "PassWord" << password.data();
        pipe.match(document.view());

        document.clear();
        document
            << "Avatar" << 1
            << "NickName" << 1
            << "CommentLikes" << 1
            << "Solves" << 1
            << "Authority" << 1;

        pipe.project(document.view());
        // 匹配账号和密码
        mongocxx::cursor cursor = usercoll.aggregate(pipe);
        // 匹配失败
        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "账户或密码错误！";
            return resjson;
        }
        // 匹配成功
        
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["Info"] = jsonvalue;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户登录】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：用户登录通过Token
    传入：Json(UserId)
    传出：Json(Result,Reason,Info(_id,NickName,Avatar,CommentLikes,Solves,Authority))
*/
Json MoDB::LoginUserByToken(Json &loginjson)
{
    Json resjson;
    try
    {
        int64_t userid = stoll(loginjson["UserId"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};
        document
            << "_id" << userid;
        pipe.match(document.view());

        document.clear();
        document
            << "Avatar" << 1
            << "NickName" << 1
            << "CommentLikes" << 1
            << "Solves" << 1
            << "Authority" << 1;

        pipe.project(document.view());

        mongocxx::cursor cursor = usercoll.aggregate(pipe);
        // 匹配失败
        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "用户ID错误！";
            return resjson;
        }
        // 匹配成功
        
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["Info"] = jsonvalue;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户登录】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：更新用户的题目信息，当用户提交代码时
    传入：Json(UserId,ProblemId,Status)
    传出：bool(如果AC是否向其添加)
*/
bool MoDB::UpdateUserProblemInfo(Json &updatejson)
{
    try
    {
        int64_t userid = stoll(updatejson["UserId"].get<std::string>());
        int problemid = stoi(updatejson["ProblemId"].get<std::string>());
        int status = stoi(updatejson["Status"].get<std::string>());

        // 将用户提交数目加一
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "SubmitNum" << 1 << close_document;

        usercoll.update_one({make_document(kvp("_id", userid))}, document.view());

        // 如果AC了
        if (status == 2)
        {
            // 查询AC题目是否已经添加至Solves的数组中
            mongocxx::pipeline pipe;
            pipe.match({make_document(kvp("_id", userid))});
            document.clear();
            document
                << "IsHasAc" << open_document
                << "$in" << open_array << problemid << "$Solves" << close_array
                << close_document;
            pipe.project(document.view());
            
            Json tmpjson;
            mongocxx::cursor cursor = usercoll.aggregate(pipe);

            for (auto doc : cursor)
            {
                tmpjson = Json::parse(bsoncxx::to_json(doc));
            }
            // 如果未添加
            if (tmpjson["IsHasAc"].get<bool>() == false)
            {
                document.clear();
                document
                    << "$push" << open_document
                    << "Solves" << problemid
                    << close_document
                    << "$inc" << open_document
                    << "ACNum" << 1 << close_document;
                usercoll.update_one({make_document(kvp("_id", userid))}, document.view());
                return true;
            }
            return false;
        }
        return false;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【更新用户的题目信息】数据库异常！" ;
        return false;
    }
}

/*
    功能：获取用户的Rank排名
    传入：Json(Page,PageSize)
    传出：Json(Result,Reason,ArrayInfo[_id,Rank,Avatar,NickName,PersonalProfile,SubmitNum,ACNum],TotalNum)
*/
Json MoDB::SelectUserRank(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];
        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe, pipetot;
        

        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = usercoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        document
            << "ACNum" << -1
            << "SubmitNum" << 1;

        pipe.sort(document.view());
        pipe.skip(skip);
        pipe.limit(pagesize);
        document.clear();
        document
            << "Avatar" << 1
            << "NickName" << 1
            << "PersonalProfile" << 1
            << "SubmitNum" << 1
            << "ACNum" << 1;
        pipe.project(document.view());

        cursor = usercoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        // 添加Rank排名
        int currank = (page - 1) * pagesize + 1;
        for (int i = 0; i < resjson["ArrayInfo"].size(); i++)
        {
            resjson["ArrayInfo"][i]["Rank"] = currank++;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【获取用户的Rank排名】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：用户主页展示
    传入：Json(UserId)
    传出：Json(Result,Reason,_id,Avatar,NickName,PersonalProfile,School,Major,JoinTime,Solves,ACNum,SubmitNum)
*/
Json MoDB::SelectUserInfo(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t userid = stoll(queryjson["UserId"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;

        pipe.match({make_document(kvp("_id", userid))});

        document
            << "Avatar" << 1
            << "NickName" << 1
            << "PersonalProfile" << 1
            << "School" << 1
            << "Major" << 1
            << "JoinTime" << 1
            << "Solves" << 1
            << "ACNum" << 1
            << "SubmitNum" << 1;
        pipe.project(document.view());

        

        mongocxx::cursor cursor = usercoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该信息！";
            return resjson;
        }

        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【用户主页展示】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：更改用户信息
    传入：Json(UserId,Avatar,PersonalProfile,School,Major)
    传出：Json(Result,Reason)
*/
Json MoDB::UpdateUserInfo(Json &updatejson)
{
    Json resjson;
    try
    {
        int64_t userid = stoll(updatejson["UserId"].get<std::string>());
        string avatar = updatejson["Avatar"].get<std::string>();
        string personalprofile = updatejson["PersonalProfile"].get<std::string>();
        string school = updatejson["School"].get<std::string>();
        string major = updatejson["Major"].get<std::string>();

        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        bsoncxx::builder::stream::document document{};
        document
            << "$set" << open_document
            << "Avatar" << avatar.data()
            << "PersonalProfile" << personalprofile.data()
            << "School" << school.data()
            << "Major" << major.data()
            << close_document;

        usercoll.update_one({make_document(kvp("_id", userid))}, document.view());

        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【更改用户信息】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：查询用户表，用于修改用户
    传入：Json(UserId)
    传出：Json(_id,Avatar,NickName,PersonalProfile,School,Major)
*/
Json MoDB::SelectUserUpdateInfo(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t userid = stoll(queryjson["UserId"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;

        pipe.match({make_document(kvp("_id", userid))});
        document
            << "Avatar" << 1
            << "NickName" << 1
            << "PersonalProfile" << 1
            << "School" << 1
            << "Major" << 1;
        pipe.project(document.view());

        

        mongocxx::cursor cursor = usercoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该信息！";
            return resjson;
        }

        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询用户表，用于修改用户】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：分页查询用户列表
    传入：Json(Page,PageSize)
    传出：Json(_id,NickName,PersonalProfile,School,Major,JoinTime,TotalNum)
*/
Json MoDB::SelectUserSetInfo(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;
        pipe.skip(skip);
        pipe.limit(pagesize);

        document
            << "NickName" << 1
            << "PersonalProfile" << 1
            << "School" << 1
            << "Major" << 1
            << "JoinTime" << 1;

        pipe.project(document.view());
        

        mongocxx::cursor cursor = usercoll.aggregate(pipe);

        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["Result"] = "Success";
        resjson["TotalNum"] = to_string(usercoll.count_documents({}));
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【分页查询用户列表】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除用户
    传入：Json(UserId)
    传出：Json(Result,Reason)
*/
Json MoDB::DeleteUser(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t userid = stoll(deletejson["UserId"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        auto result = usercoll.delete_one({make_document(kvp("_id", userid))});

        if ((*result).deleted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该数据！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除用户】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：查询所有用户的权限
    传入：void
    传出：Json(Result,_id,Authority)
*/
Json MoDB::SelectUserAuthority()
{
    Json resjson;
    try
    {
        auto client = pool.acquire();
        mongocxx::collection usercoll = (*client)["XDOJ"]["User"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;

        document
            << "Authority" << 1;

        pipe.project(document.view());
        

        mongocxx::cursor cursor = usercoll.aggregate(pipe);

        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询所有用户的权限】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：管理员查询题目信息
    传入：Json(ProblemId)
    传出：Json(Result,Reason,_id,Title,Description,TimeLimit,MemoryLimit,JudgeNum,UserNickName,Tags)
*/
Json MoDB::SelectProblemInfoByAdmin(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t problemid = stoll(queryjson["ProblemId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", problemid))});

        document
            << "Title" << 1
            << "Description" << 1
            << "TimeLimit" << 1
            << "MemoryLimit" << 1
            << "JudgeNum" << 1
            << "UserNickName" << 1
            << "Tags" << 1;
        pipe.project(document.view());

        

        mongocxx::cursor cursor = problemcoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该信息！";
            return resjson;
        }

        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【管理员查询题目信息】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：获取题目信息
    传入：Json(ProblemId)
    传出：Json(Result,Reason,_id,Title,Description,TimeLimit,MemoryLimit,JudgeNum,SubmitNum,ACNum,UserNickName,Tags)
*/
Json MoDB::SelectProblem(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t problemid = stoll(queryjson["ProblemId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", problemid))});

        document
            << "Title" << 1
            << "Description" << 1
            << "TimeLimit" << 1
            << "MemoryLimit" << 1
            << "JudgeNum" << 1
            << "SubmitNum" << 1
            << "ACNum" << 1
            << "UserNickName" << 1
            << "Tags" << 1;
        pipe.project(document.view());

        

        mongocxx::cursor cursor = problemcoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该信息！";
            return resjson;
        }

        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库出错啦！";
        LOG_ERROR << "【获取题目信息】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：插入题目
    传入：Json(Title,Description,TimeLimit,MemoryLimit,JudgeNum,Tags,UseNickName)
    传出：Json(Reuslt,Reason,ProblemId)
*/
Json MoDB::InsertProblem(Json &insertjson)
{
    Json resjson;
    try
    {
        int problemid = ++m_problemid;
        string title = insertjson["Title"].get<std::string>();
        string description = insertjson["Description"].get<std::string>();
        int timelimit = stoi(insertjson["TimeLimit"].get<std::string>());
        int memorylimit = stoi(insertjson["MemoryLimit"].get<std::string>());
        int judgenum = stoi(insertjson["JudgeNum"].get<std::string>());
        string usernickname = insertjson["UserNickName"].get<std::string>();

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];

        bsoncxx::builder::stream::document document{};
        auto in_array = document
                        << "_id" << problemid
                        << "Title" << title.data()
                        << "Description" << description.data()
                        << "TimeLimit" << timelimit
                        << "MemoryLimit" << memorylimit
                        << "JudgeNum" << judgenum
                        << "SubmitNum" << 0
                        << "CENum" << 0
                        << "ACNum" << 0
                        << "WANum" << 0
                        << "RENum" << 0
                        << "TLENum" << 0
                        << "MLENum" << 0
                        << "SENum" << 0
                        << "UserNickName" << usernickname.data()
                        << "Tags" << open_array;
        for (int i = 0; i < insertjson["Tags"].size(); i++)
        {
            string tag = insertjson["Tags"][i].get<std::string>();
            in_array = in_array << tag.data();
        }
        bsoncxx::document::value doc = in_array << close_array << finalize;
        auto result = problemcoll.insert_one(doc.view());

        if ((*result).result().inserted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库插入失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        resjson["ProblemId"] = problemid;
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【插入题目】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：修改题目信息
    传入：Json(ProblemId,Title,Description,TimeLimit,MemoryLimit,JudgeNum,Tags,UseNickName)
    传出：Json(Result,Reason)
*/
Json MoDB::UpdateProblem(Json &updatejson)
{
    Json resjson;
    try
    {
        int problemid = stoi(updatejson["ProblemId"].get<std::string>());
        string title = updatejson["Title"].get<std::string>();
        string description = updatejson["Description"].get<std::string>();
        int timelimit = stoi(updatejson["TimeLimit"].get<std::string>());
        int memorylimit = stoi(updatejson["MemoryLimit"].get<std::string>());
        int judgenum = stoi(updatejson["JudgeNum"].get<std::string>());
        string usernickname = updatejson["UserNickName"].get<std::string>();

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];

        bsoncxx::builder::stream::document document{};
        auto in_array = document
                        << "$set" << open_document
                        << "Title" << title.data()
                        << "Description" << description.data()
                        << "TimeLimit" << timelimit
                        << "MemoryLimit" << memorylimit
                        << "JudgeNum" << judgenum
                        << "UserNickName" << usernickname.data()
                        << "Tags" << open_array;
        for (int i = 0; i < updatejson["Tags"].size(); i++)
        {
            string tag = updatejson["Tags"][i].get<std::string>();
            in_array = in_array << tag.data();
        }
        bsoncxx::document::value doc = in_array << close_array << close_document << finalize;

        problemcoll.update_one({make_document(kvp("_id", problemid))}, doc.view());

        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【修改题目信息】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除题目
    传入：Json(ProblemId)
    传出：Json(Result,Reason)
*/
Json MoDB::DeleteProblem(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t problemid = stoll(deletejson["ProblemId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];

        auto result = problemcoll.delete_one({make_document(kvp("_id", problemid))});

        if ((*result).deleted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该数据！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除题目】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：将字符串变为数字
    主要为查询ID服务，限制：ID长度不能大于4，只关注数字
*/
int mystoi(string num)
{
    int resnum = 0;
    if (num.size() >= 4 || num.size() == 0)
        return resnum;
    for (auto n : num)
    {
        if (isdigit(n))
        {
            resnum = resnum * 10 + n - '0';
        }
    }
    return resnum;
}

/*
    功能：将字符串变为int64
    主要为查询ID服务，限制：ID长度不能大于19，只关注数字
*/
int64_t mystoll(string num)
{
    int64_t resnum = 0;
    if (num.size() >= 19 || num.size() == 0)
        return resnum;
    for (auto n : num)
    {
        if (isdigit(n))
        {
            resnum = resnum * 10 + n - '0';
        }
    }
    return resnum;
}
/*
    功能：分页获取题目列表
    前端传入
    Json(SearchInfo,Page,PageSize)
    后端传出
    Json(([ProblemId,Title,SubmitNum,CENum,ACNum,WANum,RENum,TLENum,MLENum,SENum,Tags]),TotalNum)
*/
Json MoDB::SelectProblemList(Json &queryjson)
{
    Json resjson;
    try
    {
        Json searchinfo = queryjson["SearchInfo"];
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];
        
        mongocxx::pipeline pipe, pipetot;
        bsoncxx::builder::stream::document document{};

        // 查询ID
        if (searchinfo["Id"].get<std::string>().size() > 0)
        {
            int id = mystoi(searchinfo["Id"].get<std::string>());
            pipe.match({make_document(kvp("_id", id))});
            pipetot.match({make_document(kvp("_id", id))});
        }

        // 查询标题
        if (searchinfo["Title"].get<std::string>().size() > 0)
        {
            document
                << "Title" << open_document
                << "$regex" << searchinfo["Title"].get<std::string>()
                << close_document;
            pipe.match(document.view());
            pipetot.match(document.view());
            document.clear();
        }

        // 查询标签
        if (searchinfo["Tags"].size() > 0)
        {
            auto in_array = document
                            << "Tags" << open_document
                            << "$in" << open_array;
            for (int i = 0; i < searchinfo["Tags"].size(); i++)
            {
                in_array = in_array
                           << searchinfo["Tags"][i].get<std::string>();
            }
            bsoncxx::document::value doc = in_array << close_array << close_document << finalize;
            pipe.match(doc.view());
            pipetot.match(doc.view());
            document.clear();
        }
        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = problemcoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        // 排序
        pipe.sort({make_document(kvp("_id", 1))});
        // 跳过
        pipe.skip(skip);
        // 限制
        pipe.limit(pagesize);
        // 进行
        document
            << "ProblemId"
            << "$_id"
            << "Title" << 1
            << "SubmitNum" << 1
            << "CENum" << 1
            << "ACNum" << 1
            << "WANum" << 1
            << "RENum" << 1
            << "TLENum" << 1
            << "MLENum" << 1
            << "SENum" << 1
            << "Tags" << 1;
        pipe.project(document.view());

        cursor = problemcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["Reuslt"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库出错啦！";
        LOG_ERROR << "【分页获取题目列表】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：管理员分页获取题目列表
    传入：Json(Page,PageSize)
    传出：Json(ArrayInfo([ProblemId,Title,SubmitNum,CENum,ACNum,WANum,RENum,TLENum,MLENum,SENum,Tags]),TotalNum)
*/
Json MoDB::SelectProblemListByAdmin(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];

        
        mongocxx::pipeline pipe, pipetot;
        bsoncxx::builder::stream::document document{};

        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = problemcoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        // 排序
        pipe.sort({make_document(kvp("_id", 1))});
        // 跳过
        pipe.skip(skip);
        // 限制
        pipe.limit(pagesize);
        // 进行
        document
            << "ProblemId"
            << "$_id"
            << "Title" << 1
            << "SubmitNum" << 1
            << "CENum" << 1
            << "ACNum" << 1
            << "WANum" << 1
            << "RENum" << 1
            << "TLENum" << 1
            << "MLENum" << 1
            << "SENum" << 1
            << "Tags" << 1;
        pipe.project(document.view());

        Json arryjson;
        cursor = problemcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            arryjson.push_back(jsonvalue);
        }
        resjson["ArrayInfo"] = arryjson;
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库出错啦！";
        LOG_ERROR << "【管理员分页获取题目列表】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：更新题目的状态数量
    传入：Json(ProblemId,Status)
    传出：bool
*/
bool MoDB::UpdateProblemStatusNum(Json &updatejson)
{
    try
    {
        int64_t problemid = stoll(updatejson["ProblemId"].get<std::string>());
        int status = stoi(updatejson["Status"].get<std::string>());

        string statusnum = "";
        if (status == 1)
            statusnum = "CENum";
        else if (status == 2)
            statusnum = "ACNum";
        else if (status == 3)
            statusnum = "WANum";
        else if (status == 4)
            statusnum = "RENum";
        else if (status == 5)
            statusnum = "TLENum";
        else if (status == 6)
            statusnum = "MLENum";
        else if (status == 7)
            statusnum = "SENum";

        auto client = pool.acquire();
        mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];
        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "SubmitNum" << 1
            << statusnum << 1 << close_document;

        problemcoll.update_one({make_document(kvp("_id", problemid))}, document.view());
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【更新题目的状态数量】数据库异常！" ;
        return false;
    }
}

/*
    功能：获取题目的所有标签
    传入：void
    传出：Json(tags)
*/
Json MoDB::getProblemTags()
{
    auto client = pool.acquire();
    mongocxx::collection problemcoll = (*client)["XDOJ"]["Problem"];
    mongocxx::cursor cursor = problemcoll.distinct({"Tags"}, {});
    
    Json resjson;
    for (auto doc : cursor)
    {
        resjson = Json::parse(bsoncxx::to_json(doc));
    }
    return resjson;
}
/*
    功能：插入待测评记录
    传入：Json(ProblemId,UserId,UserNickName,ProblemTitle,Language,Code)
    传出：SubmitId测评的ID
*/
string MoDB::InsertStatusRecord(Json &insertjson)
{
    try
    {
        int64_t id = ++m_statusrecordid;
        int64_t problemid = stoll(insertjson["ProblemId"].get<std::string>());
        int64_t userid = stoll(insertjson["UserId"].get<std::string>());
        string usernickname = insertjson["UserNickName"].get<std::string>();
        string problemtitle = insertjson["ProblemTitle"].get<std::string>();
        string language = insertjson["Language"].get<std::string>();
        string code = insertjson["Code"].get<std::string>();

        auto client = pool.acquire();
        mongocxx::collection statusrecordcoll = (*client)["XDOJ"]["StatusRecord"];
        bsoncxx::builder::stream::document document{};

        document
            << "_id" << id
            << "ProblemId" << problemid
            << "UserId" << userid
            << "UserNickName" << usernickname.data()
            << "ProblemTitle" << problemtitle.data()
            << "Status" << 0
            << "RunTime"
            << "0MS"
            << "RunMemory"
            << "0MB"
            << "Length"
            << "0B"
            << "Language" << language.data()
            << "SubmitTime" << GetTime().data()
            << "Code" << code.data()
            << "ComplierInfo"
            << ""
            << "TestInfo" << open_array << close_array;
        statusrecordcoll.insert_one(document.view());
        return to_string(id);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【插入待测评记录】数据库异常！" ;
        return "0";
    }
}

/*
    功能：更新测评记录
    传入：Json(SubmitId,Status,RunTime,RunMemory,Length,ComplierInfo,
    TestInfo[(Status,StandardInput,StandardOutput,PersonalOutput,RunTime,RunMemory)])
    传出：bool
*/
bool MoDB::UpdateStatusRecord(Json &updatejson)
{
    Json resjson;
    try
    {
        int64_t submitid = stoll(updatejson["SubmitId"].get<std::string>());
        int status = stoi(updatejson["Status"].get<std::string>());
        string runtime = updatejson["RunTime"].get<std::string>();
        string runmemory = updatejson["RunMemory"].get<std::string>();
        string length = updatejson["Length"].get<std::string>();
        string complierinfo = updatejson["ComplierInfo"].get<std::string>();

        // 更新测评记录
        auto client = pool.acquire();
        mongocxx::collection statusrecordcoll = (*client)["XDOJ"]["StatusRecord"];
        bsoncxx::builder::stream::document document{};
        auto in_array = document
                        << "$set" << open_document
                        << "Status" << status
                        << "RunTime" << runtime.data()
                        << "RunMemory" << runmemory.data()
                        << "Length" << length.data()
                        << "ComplierInfo" << complierinfo.data()
                        << "TestInfo" << open_array;

        for (int i = 0; i < updatejson["TestInfo"].size(); i++)
        {
            int teststatus = stoi(updatejson["TestInfo"][i]["Status"].get<std::string>());
            string standardinput = updatejson["TestInfo"][i]["StandardInput"].get<std::string>();
            string standardoutput = updatejson["TestInfo"][i]["StandardOutput"].get<std::string>();
            string personaloutput = updatejson["TestInfo"][i]["PersonalOutput"].get<std::string>();
            string testruntime = updatejson["TestInfo"][i]["RunTime"].get<std::string>();
            string testrunmemory = updatejson["TestInfo"][i]["RunMemory"].get<std::string>();
            in_array = in_array << open_document
                                << "Status" << teststatus
                                << "StandardInput" << standardinput
                                << "StandardOutput" << standardoutput
                                << "PersonalOutput" << personaloutput
                                << "RunTime" << testruntime
                                << "RunMemory" << testrunmemory << close_document;
        }
        bsoncxx::document::value doc = in_array << close_array << close_document << finalize;

        statusrecordcoll.update_one({make_document(kvp("_id", submitid))}, doc.view());

        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【更新测评记录】数据库异常！" ;
        return false;
    }
}

/*
    功能：分页查询测评记录
    传入：Json(SearchInfo,PageSize,Page)
    传出：测评全部信息，详情请见MongoDB集合表
*/
Json MoDB::SelectStatusRecordList(Json &queryjson)
{
    Json resjson;
    try
    {
        Json searchinfo = queryjson["SearchInfo"];
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        
        mongocxx::pipeline pipe, pipetot;
        bsoncxx::builder::stream::document document{};
        auto client = pool.acquire();
        mongocxx::collection statusrecordcoll = (*client)["XDOJ"]["StatusRecord"];

        // 查询题目ID
        if (searchinfo["ProblemId"].get<std::string>().size() > 0)
        {
            int64_t problemid = mystoll(searchinfo["ProblemId"].get<std::string>());
            pipe.match({{make_document(kvp("ProblemId", problemid))}});
            pipetot.match({{make_document(kvp("ProblemId", problemid))}});
        }
        // 查询用户ID
        if (searchinfo["UserId"].get<std::string>().size() > 0)
        {
            int64_t userid = stoll(searchinfo["UserId"].get<std::string>());
            pipe.match({{make_document(kvp("UserId", userid))}});
            pipetot.match({{make_document(kvp("UserId", userid))}});
        }

        // 查询题目标题
        if (searchinfo["ProblemTitle"].get<std::string>().size() > 0)
        {
            document
                << "ProblemTitle" << open_document
                << "$regex" << searchinfo["ProblemTitle"].get<std::string>()
                << close_document;
            pipe.match(document.view());
            pipetot.match(document.view());
            document.clear();
        }

        // 查询状态
        if (searchinfo["Status"].get<std::string>().size() > 0)
        {
            int status = stoi(searchinfo["Status"].get<std::string>());
            pipe.match({{make_document(kvp("Status", status))}});
            pipetot.match({{make_document(kvp("Status", status))}});
        }

        // 查询语言
        if (searchinfo["Language"].get<std::string>().size() > 0)
        {
            string language = searchinfo["Language"].get<std::string>();
            pipe.match({{make_document(kvp("Language", language))}});
            pipetot.match({{make_document(kvp("Language", language))}});
        }
        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = statusrecordcoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        // 排序
        pipe.sort({make_document(kvp("SubmitTime", -1))});
        // 跳过
        pipe.skip(skip);
        // 限制
        pipe.limit(pagesize);

        document
            << "ProbleId" << 1
            << "UserId" << 1
            << "UserNickName" << 1
            << "ProblemTitle" << 1
            << "Status" << 1
            << "RunTime" << 1
            << "RunMemory" << 1
            << "Length" << 1
            << "Language" << 1
            << "SubmitTime" << 1;
        pipe.project(document.view());
        Json arryjson;
        cursor = statusrecordcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            arryjson.push_back(jsonvalue);
        }
        resjson["ArrayInfo"] = arryjson;
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【分页查询测评记录】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：查询测评记录
    传入：Json(SubmitId)
    传出：全部记录，详情请看MongoDB集合表
*/
Json MoDB::SelectStatusRecord(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t submitid = stoll(queryjson["SubmitId"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection statusrecordcoll = (*client)["XDOJ"]["StatusRecord"];

        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};

        pipe.match({make_document(kvp("_id", submitid))});

        document
            << "Status" << 1
            << "Language" << 1
            << "Code" << 1
            << "ComplierInfo" << 1
            << "TestInfo" << 1;

        pipe.project(document.view());
        // 查询测评记录
        

        mongocxx::cursor cursor = statusrecordcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";

        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询测评记录】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：添加讨论
    传入：Json(Title,Content,ParentId,UserId) 如果是父讨论ParentId=0
    传出：Json(Result)
*/
Json MoDB::InsertDiscuss(Json &insertjson)
{
    Json resjson;
    try
    {
        int64_t id = ++m_articleid;
        string title = insertjson["Title"].get<std::string>();
        string content = insertjson["Content"].get<std::string>();
        int64_t parentid = atoll(insertjson["ParentId"].get<std::string>().data());
        int64_t userid = atoll(insertjson["UserId"].get<std::string>().data());

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];
        bsoncxx::builder::stream::document document{};
        document
            << "_id" << id
            << "Title" << title.data()
            << "Content" << content.data()
            << "ParentId" << parentid
            << "UserId" << userid
            << "Views" << 0
            << "Comments" << 0
            << "CreateTime" << GetTime().data()
            << "UpdateTime" << GetTime().data();

        auto result = discusscoll.insert_one(document.view());

        if ((*result).result().inserted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库插入失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【添加讨论】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：分页查询讨论
    传入：Json(SearchInfo,Page,PageSize)
    传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
*/
Json MoDB::SelectDiscussList(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t parentid = stoll(queryjson["SearchInfo"]["ParentId"].get<std::string>());
        int64_t userid = stoll(queryjson["SearchInfo"]["UserId"].get<std::string>());
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        
        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe, pipetot;

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];

        if (parentid > 0)
        {
            pipe.match({make_document(kvp("ParentId", parentid))});
            pipetot.match({make_document(kvp("ParentId", parentid))});
        }

        if (userid > 0)
        {
            pipe.match({make_document(kvp("UserId", userid))});
            pipetot.match({make_document(kvp("UserId", userid))});
        }

        if (parentid == 0 && userid == 0)
        {
            pipe.match({make_document(kvp("ParentId", 0))});
            pipetot.match({make_document(kvp("ParentId", 0))});
        }
        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = discusscoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        pipe.sort({make_document(kvp("CreateTime", -1))});
        pipe.skip(skip);
        pipe.limit(pagesize);
        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());

        document.clear();
        document
            << "Title" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1;
        pipe.project(document.view());

        cursor = discusscoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【分页查询讨论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：管理员分页查询讨论
    传入：Json(Page,PageSize)
    传出：Json(_id,Title,Views,Comments,CreateTime,UserId,User.Avatar,User.NickName)
*/
Json MoDB::SelectDiscussListByAdmin(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        
        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe, pipetot;

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];
        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = discusscoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        pipe.sort({make_document(kvp("CreateTime", -1))});
        pipe.skip(skip);
        pipe.limit(pagesize);
        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());

        document.clear();
        document
            << "Title" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "UserId" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1;
        pipe.project(document.view());

        cursor = discusscoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【管理员分页查询讨论】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：查询讨论的详细信息，主要是编辑时的查询
    传入：Json(DiscussId)
    传出：Json(Result,Reason,Title,Content)
*/
Json MoDB::SelectDiscussByEdit(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t discussid = stoll(queryjson["DiscussId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", discussid))});
        document
            << "Title" << 1
            << "UserId" << 1
            << "Content" << 1;
        pipe.project(document.view());
        mongocxx::cursor cursor = discusscoll.aggregate(pipe);

        

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据！";
            return resjson;
        }
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询讨论的详细信息，主要是编辑时的查询】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：查询讨论的详细内容，并且将其浏览量加一
    传入：Json(DiscussId)
    传出：Json(Resutl,Reason,Content,Views,Comments,CreateTime,UpdateTime,User.NickName,User.Avatar)
*/
Json MoDB::SelectDiscuss(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t discussid = stoll(queryjson["DiscussId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];
        // 浏览量加一
        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "Views" << 1 << close_document;
        discusscoll.update_one({make_document(kvp("_id", discussid))}, document.view());

        // 查询Content
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", discussid))});
        document.clear();
        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());

        document.clear();
        document
            << "Title" << 1
            << "Content" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "UpdateTime" << 1
            << "User._id" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1;
        pipe.project(document.view());
        mongocxx::cursor cursor = discusscoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据，可能是请求参数错误！";
            return resjson;
        }

        
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询讨论的详细内容，并且将其浏览量加一】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：修改讨论的评论数
    传入：Json(ArticleId,Num)
    传出：bool
*/
bool MoDB::UpdateDiscussComments(Json &updatejson)
{
    try
    {
        int64_t discussid = stoll(updatejson["ArticleId"].get<std::string>());
        int num = stoi(updatejson["Num"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];

        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "Comments" << num << close_document;
        discusscoll.update_one({make_document(kvp("_id", discussid))}, document.view());
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【修改讨论的评论数】数据库异常！" ;
        return false;
    }
}

/*
    功能：更新讨论
    传入：Json(DiscussId,Title,Content)
    传出；Json(Result,Reason)
*/
Json MoDB::UpdateDiscuss(Json &updatejson)
{
    Json resjson;
    try
    {
        int64_t discussid = stoll(updatejson["DiscussId"].get<std::string>());
        string title = updatejson["Title"].get<std::string>();
        string content = updatejson["Content"].get<std::string>();

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];

        bsoncxx::builder::stream::document document{};
        document
            << "$set" << open_document
            << "Title" << title.data()
            << "Content" << content.data()
            << "UpdateTime" << GetTime().data()
            << close_document;

        discusscoll.update_one({make_document(kvp("_id", discussid))}, document.view());

        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【更新讨论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除讨论
    传入：Json(DiscussId)
    传出：Json(Result,Reason)
*/
Json MoDB::DeleteDiscuss(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t discussid = stoll(deletejson["DiscussId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection discusscoll = (*client)["XDOJ"]["Discuss"];

        auto result = discusscoll.delete_one({make_document(kvp("_id", discussid))});

        if ((*result).deleted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库删除失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除讨论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：添加题解
    传入：Json(Title,Content,ParentId,UserId,Public)
    传出：Json(Result,Reason)
*/
Json MoDB::InsertSolution(Json &insertjson)
{
    Json resjson;
    try
    {
        int64_t id = ++m_articleid;
        string title = insertjson["Title"].get<std::string>();
        string content = insertjson["Content"].get<std::string>();
        int64_t parentid = stoll(insertjson["ParentId"].get<std::string>());
        int64_t userid = stoll(insertjson["UserId"].get<std::string>());
        bool ispublic = insertjson["Public"].get<bool>();

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];
        bsoncxx::builder::stream::document document{};
        document
            << "_id" << id
            << "Title" << title.data()
            << "Content" << content.data()
            << "ParentId" << parentid
            << "UserId" << userid
            << "Views" << 0
            << "Comments" << 0
            << "Public" << ispublic
            << "CreateTime" << GetTime().data()
            << "UpdateTime" << GetTime().data();

        auto result = solutioncoll.insert_one(document.view());

        if ((*result).result().inserted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库插入失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【添加题解】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：分页查询题解（公开题解）
    传入：Json(SearchInfo,Page,PageSize)
    传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
*/
Json MoDB::SelectSolutionList(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t parentid = stoll(queryjson["SearchInfo"]["ParentId"].get<std::string>());
        int64_t userid = stoll(queryjson["SearchInfo"]["UserId"].get<std::string>());
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        
        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe, pipetot;

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];

        if (parentid > 0)
        {
            // 匹配ID
            pipetot.match({make_document(kvp("ParentId", parentid))});
            pipe.match({make_document(kvp("ParentId", parentid))});

            // 匹配公开
            pipetot.match({make_document(kvp("Public", true))});
            pipe.match({make_document(kvp("Public", true))});
        }
        if (userid > 0)
        {
            // 匹配ID
            pipetot.match({make_document(kvp("UserId", userid))});
            pipe.match({make_document(kvp("UserId", userid))});
        }
        // 获取总条数

        pipetot.count("TotalNum");
        mongocxx::cursor cursor = solutioncoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        pipe.sort({make_document(kvp("CreateTime", -1))});
        pipe.skip(skip);
        pipe.limit(pagesize);
        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());

        document.clear();
        document
            << "Title" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1;
        pipe.project(document.view());

        cursor = solutioncoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【分页查询题解（公开题解）】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：管理员分页查询题解
    传入：Json(Page,PageSize)
    传出：Json(_id,Title,Views,Comments,CreateTime,User.Avatar,User.NickName)
*/
Json MoDB::SelectSolutionListByAdmin(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        
        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe, pipetot;

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];

        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = solutioncoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }

        pipe.sort({make_document(kvp("CreateTime", -1))});
        pipe.skip(skip);
        pipe.limit(pagesize);
        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());

        document.clear();
        document
            << "Title" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "UserId" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1;
        pipe.project(document.view());

        cursor = solutioncoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【管理员分页查询题解】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：查询题解的详细信息，主要是编辑时的查询
    传入：Json(SolutionId)
    传出：Json(Result,Reason,Title,Content,UserId,Public)
*/
Json MoDB::SelectSolutionByEdit(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t solutionid = stoll(queryjson["SolutionId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", solutionid))});
        document
            << "Title" << 1
            << "Content" << 1
            << "UserId" << 1
            << "Public" << 1;
        pipe.project(document.view());
        mongocxx::cursor cursor = solutioncoll.aggregate(pipe);

        
        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据！";
            return resjson;
        }
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询题解的详细信息，主要是编辑时的查询】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：查询题解的详细内容，并且将其浏览量加一
    传入：Json(SolutionId)
    传出：Json(Result,Reason,Title,Content,Views,Comments,CreateTime,UpdateTime,User.NicaName,User.Avatar)
*/
Json MoDB::SelectSolution(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t solutionid = stoll(queryjson["SolutionId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];
        // 浏览量加一
        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "Views" << 1 << close_document;
        solutioncoll.update_one({make_document(kvp("_id", solutionid))}, document.view());

        // 查询Content
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", solutionid))});
        document.clear();

        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());

        document.clear();
        document
            << "Title" << 1
            << "Content" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "UpdateTime" << 1
            << "User._id" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1;
        pipe.project(document.view());

        mongocxx::cursor cursor = solutioncoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据，可能是请求参数错误！";
            return resjson;
        }
        
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询题解的详细内容，并且将其浏览量加一】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：修改题解的评论数
    传入：Json(ArticleId,Num)
    传出：bool
*/
bool MoDB::UpdateSolutionComments(Json &updatejson)
{
    try
    {
        int64_t articleid = stoll(updatejson["ArticleId"].get<std::string>());
        int num = stoi(updatejson["Num"].get<std::string>());
        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];

        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "Comments" << num << close_document;
        solutioncoll.update_one({make_document(kvp("_id", articleid))}, document.view());
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【修改题解的评论数】数据库异常！" ;
        return false;
    }
}

/*
    功能：更新题解
    传入：Json(SolutionId,Title,Content,Public)
    传出；Json(Result,Reason)
*/
Json MoDB::UpdateSolution(Json &updatejson)
{
    Json resjson;
    try
    {
        int64_t solutionid = stoll(updatejson["SolutionId"].get<std::string>());
        string title = updatejson["Title"].get<std::string>();
        string content = updatejson["Content"].get<std::string>();
        bool ispublic = updatejson["Public"].get<bool>();

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];

        bsoncxx::builder::stream::document document{};
        document
            << "$set" << open_document
            << "Title" << title.data()
            << "Content" << content.data()
            << "Public" << ispublic
            << "UpdateTime" << GetTime().data()
            << close_document;

        solutioncoll.update_one({make_document(kvp("_id", solutionid))}, document.view());

        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【更新题解】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除题解
    传入：Json(SolutionId)
    传出：Json(Result,Reason)
*/
Json MoDB::DeleteSolution(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t solutionid = stoll(deletejson["SolutionId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection solutioncoll = (*client)["XDOJ"]["Solution"];

        auto result = solutioncoll.delete_one({make_document(kvp("_id", solutionid))});

        if ((*result).deleted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库删除失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除题解】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：添加公告
    传入：Json(Title,Content,UserId,Level)
    传出：Json(Result,Reason)
*/
Json MoDB::InsertAnnouncement(Json &insertjson)
{
    Json resjson;
    try
    {
        int64_t id = ++m_articleid;
        string title = insertjson["Title"].get<std::string>();
        string content = insertjson["Content"].get<std::string>();
        int64_t userid = stoll(insertjson["UserId"].get<std::string>());
        int level = stoi(insertjson["Level"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];
        bsoncxx::builder::stream::document document{};
        document
            << "_id" << id
            << "Title" << title.data()
            << "Content" << content.data()
            << "UserId" << userid
            << "Views" << 0
            << "Comments" << 0
            << "Level" << level
            << "CreateTime" << GetTime().data()
            << "UpdateTime" << GetTime().data();

        auto result = announcementcoll.insert_one(document.view());
        if ((*result).result().inserted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库插入失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【添加公告】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：分页查询公告
    传入：Json(Page,PageSize)
    传出：Json([Result,Reason,_id,Title,Views,Comments,CreateTime],TotalNum)
*/
Json MoDB::SelectAnnouncementList(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        
        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe, pipetot;

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];

        // 获取总条数
        pipetot.count("TotalNum");
        mongocxx::cursor cursor = announcementcoll.aggregate(pipetot);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        pipe.sort({make_document(kvp("CreateTime", -1))});
        pipe.sort({make_document(kvp("Level", -1))});
        pipe.skip(skip);
        pipe.limit(pagesize);

        document
            << "Title" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1;
        pipe.project(document.view());

        cursor = announcementcoll.aggregate(pipe);

        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【分页查询公告】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：查询公告的详细信息，主要是编辑时的查询
    传入：Json(AnnouncementId)
    传出：Json(Result,Reason,Title,Content,Level)
*/
Json MoDB::SelectAnnouncementByEdit(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t announcementid = stoll(queryjson["AnnouncementId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];

        bsoncxx::builder::stream::document document{};
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", announcementid))});
        document
            << "Title" << 1
            << "Content" << 1
            << "Level" << 1;
        pipe.project(document.view());
        mongocxx::cursor cursor = announcementcoll.aggregate(pipe);

        
        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据！,可能是请求参数出错！";
            return resjson;
        }
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询公告的详细信息，主要是编辑时的查询】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：查询公告的详细内容，并且将其浏览量加一
    传入：Json(AnnouncementId)
    传出：Json(Title,Content,Views,Comments,CreateTime,UpdateTime)
*/
Json MoDB::SelectAnnouncement(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t announcementid = stoll(queryjson["AnnouncementId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];
        // 浏览量加一
        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "Views" << 1 << close_document;
        announcementcoll.update_one({make_document(kvp("_id", announcementid))}, document.view());

        // 查询
        mongocxx::pipeline pipe;
        pipe.match({make_document(kvp("_id", announcementid))});
        document.clear();
        document
            << "Title" << 1
            << "Content" << 1
            << "Views" << 1
            << "Comments" << 1
            << "CreateTime" << 1
            << "UpdateTime" << 1;
        pipe.project(document.view());
        mongocxx::cursor cursor = announcementcoll.aggregate(pipe);

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据，可能是请求参数出错！";
            return resjson;
        }

        
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询公告的详细内容，并且将其浏览量加一】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：修改公告的评论数
    传入：Json(ArticleId,Num)
    传出：bool
*/
bool MoDB::UpdateAnnouncementComments(Json &updatejson)
{
    try
    {
        int64_t articleid = stoll(updatejson["ArticleId"].get<std::string>());
        int num = stoi(updatejson["Num"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];

        bsoncxx::builder::stream::document document{};
        document
            << "$inc" << open_document
            << "Comments" << num << close_document;
        announcementcoll.update_one({make_document(kvp("_id", articleid))}, document.view());
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【修改公告的评论数】数据库异常！" ;
        return false;
    }
}

/*
    功能：更新公告
    传入：Json(AnnouncementId,Title,Content,Level)
    传出；Json(Result,Reason)
*/
Json MoDB::UpdateAnnouncement(Json &updatejson)
{
    Json resjson;
    try
    {
        int64_t announcementid = stoll(updatejson["AnnouncementId"].get<std::string>());
        string title = updatejson["Title"].get<std::string>();
        string content = updatejson["Content"].get<std::string>();
        int level = stoi(updatejson["Level"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];

        bsoncxx::builder::stream::document document{};
        document
            << "$set" << open_document
            << "Title" << title.data()
            << "Content" << content.data()
            << "Level" << level
            << "UpdateTime" << GetTime().data()
            << close_document;

        announcementcoll.update_one({make_document(kvp("_id", announcementid))}, document.view());

        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【更新公告】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除公告
    传入：Json(AnnouncementId)
    传出：Json(Result,Reason)
*/
Json MoDB::DeleteAnnouncement(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t announcementid = stoll(deletejson["AnnouncementId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection announcementcoll = (*client)["XDOJ"]["Announcement"];

        auto result = announcementcoll.delete_one({make_document(kvp("_id", announcementid))});
        if ((*result).deleted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库删除失败！";
            return resjson;
        }
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除公告】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：管理员查询评论
    传入：Json(Page,PageSize)
    传出：Json(_id,ParentId,ParentType,Content,CreateTime,
        Child_Comments._id,Child_Comments.Content,Child_Comments.CreateTime)
*/
Json MoDB::SelectCommentListByAdmin(Json &queryjson)
{
    Json resjson;
    try
    {
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];

        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};
        // 按照时间先后顺序
        pipe.sort({make_document(kvp("CreateTime", -1))});
        // 跳过多少条
        pipe.skip(skip);
        // 限制多少条
        pipe.limit(pagesize);

        // 选择需要的字段
        document
            << "ParentId" << 1
            << "ParentType" << 1
            << "Content" << 1
            << "CreateTime" << 1
            << "Child_Comments._id" << 1
            << "Child_Comments.Content" << 1
            << "Child_Comments.CreateTime" << 1;
        pipe.project(document.view());
        document.clear();

        

        mongocxx::cursor cursor = commentcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArrayInfo"].push_back(jsonvalue);
        }
        resjson["TotalNum"] = to_string(commentcoll.count_documents({}));
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【管理员查询评论】数据库异常！" ;
        return resjson;
    }
}
/*
    功能：查询父评论
    传入：Json(ParentId,Page,PageSize,SonNum)
    传出：
    Json(ParentId,Content,Likes,CreateTime,Child_Total,
    User(Avatar,NickName),
    Child_Comments(_id,Content,Likes,CreateTime,User(Avatar,NickName)))
*/
Json MoDB::getFatherComment(Json &queryjson)
{
    Json resjson;
    try
    {
        int64_t parentid = stoll(queryjson["ParentId"].get<std::string>());
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int sonnum = stoi(queryjson["SonNum"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];

        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};
        // 匹配ParentId
        pipe.match({make_document(kvp("ParentId", parentid))});
        // 按照时间先后顺序
        pipe.sort({make_document(kvp("CreateTime", 1))});
        // 跳过多少条
        pipe.skip(skip);
        // 限制多少条
        pipe.limit(pagesize);
        // 将子评论进行求个数
        document
            << "$set" << open_document
            << "Child_Total" << open_document
            << "$size"
            << "$Child_Comments"
            << close_document << close_document;

        pipe.append_stage(document.view());
        document.clear();
        // 限制子评论的个数
        document
            << "$set" << open_document
            << "Child_Comments" << open_document
            << "$slice" << open_array
            << "$Child_Comments" << 0 << sonnum << close_array
            << close_document << close_document;

        pipe.append_stage(document.view());
        document.clear();
        // 将数组拆散
        document
            << "path"
            << "$Child_Comments"
            << "preserveNullAndEmptyArrays" << true;
        pipe.unwind(document.view());
        document.clear();
        // 将子评论的用户id和用户表进行外连接
        document
            << "from"
            << "User"
            << "localField"
            << "Child_Comments.UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "Child_Comments.User";
        pipe.lookup(document.view());
        document.clear();
        // 将其合并
        document
            << "_id"
            << "$_id"
            << "ParentId" << open_document
            << "$first"
            << "$ParentId" << close_document
            << "Content" << open_document
            << "$first"
            << "$Content" << close_document
            << "Likes" << open_document
            << "$first"
            << "$Likes" << close_document
            << "UserId" << open_document
            << "$first"
            << "$UserId" << close_document
            << "CreateTime" << open_document
            << "$first"
            << "$CreateTime" << close_document
            << "Child_Total" << open_document
            << "$first"
            << "$Child_Total" << close_document
            << "Child_Comments" << open_document
            << "$push"
            << "$Child_Comments" << close_document;
        pipe.group(document.view());
        document.clear();
        // 将父评论的用户Id和用户表进行外连接
        document
            << "from"
            << "User"
            << "localField"
            << "UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "User";
        pipe.lookup(document.view());
        document.clear();
        // 选择需要的字段
        document
            << "ParentId" << 1
            << "Content" << 1
            << "Likes" << 1
            << "CreateTime" << 1
            << "Child_Total" << 1
            << "User._id" << 1
            << "User.Avatar" << 1
            << "User.NickName" << 1
            << "Child_Comments._id" << 1
            << "Child_Comments.Content" << 1
            << "Child_Comments.Likes" << 1
            << "Child_Comments.CreateTime" << 1
            << "Child_Comments.User._id" << 1
            << "Child_Comments.User.Avatar" << 1
            << "Child_Comments.User.NickName" << 1;
        pipe.project(document.view());
        document.clear();
        // 按照时间先后顺序
        pipe.sort({make_document(kvp("CreateTime", 1))});

        

        mongocxx::cursor cursor = commentcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            Json jsonvalue;
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
            resjson["ArryInfo"].push_back(jsonvalue);
        }
        resjson["TotalNum"] = to_string(commentcoll.count_documents({make_document(kvp("ParentId", parentid))}));
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【查询父评论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：获取子评论
    传入：Json(ParentId,Skip,Limit)
    传出：Json(Child_Total,Child_Comments(_id,Content,Likes,CreateTime,User(NickName,Avatar)))
*/
Json MoDB::getSonComment(Json &queryjson)
{
    Json resjson;

    try
    {
        int64_t _id = stoll(queryjson["ParentId"].get<std::string>());
        int page = stoi(queryjson["Page"].get<std::string>());
        int pagesize = stoi(queryjson["PageSize"].get<std::string>());
        int skip = (page - 1) * pagesize;

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];
        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};
        // 匹配id
        pipe.match({make_document(kvp("_id", _id))});

        // 将子评论进行求个数
        document
            << "$set" << open_document
            << "Child_Total" << open_document
            << "$size"
            << "$Child_Comments"
            << close_document << close_document;

        pipe.append_stage(document.view());
        document.clear();
        // 限制子评论个数
        document
            << "$set" << open_document
            << "Child_Comments" << open_document
            << "$slice" << open_array
            << "$Child_Comments" << skip << pagesize << close_array
            << close_document << close_document;

        pipe.append_stage(document.view());
        document.clear();
        // 将数组拆散
        document
            << "path"
            << "$Child_Comments"
            << "preserveNullAndEmptyArrays" << true;
        pipe.unwind(document.view());
        document.clear();
        // 将子评论的用户id和用户表进行外连接
        document
            << "from"
            << "User"
            << "localField"
            << "Child_Comments.UserId"
            << "foreignField"
            << "_id"
            << "as"
            << "Child_Comments.User";
        pipe.lookup(document.view());
        document.clear();

        // 将其合并
        document
            << "_id"
            << "$_id"
            << "Child_Total" << open_document
            << "$first"
            << "$Child_Total" << close_document
            << "Child_Comments" << open_document
            << "$push"
            << "$Child_Comments" << close_document;
        pipe.group(document.view());
        document.clear();
        document
            << "Child_Total" << 1
            << "Child_Comments._id" << 1
            << "Child_Comments.Content" << 1
            << "Child_Comments.Likes" << 1
            << "Child_Comments.CreateTime" << 1
            << "Child_Comments.User._id" << 1
            << "Child_Comments.User.NickName" << 1
            << "Child_Comments.User.Avatar" << 1;
        pipe.project(document.view());
        document.clear();

        

        mongocxx::cursor cursor = commentcoll.aggregate(pipe);
        for (auto doc : cursor)
        {
            resjson = Json::parse(bsoncxx::to_json(doc));
        }
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【获取子评论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：插入父评论
    传入：Json(ParentId,Content,UserId)
    传出：Json(_id,CreateTime)
*/
Json MoDB::InsertFatherComment(Json &insertjson)
{
    Json resjson;
    try
    {
        int64_t id = ++m_commentid;
        int64_t parentid = stoll(insertjson["ParentId"].get<std::string>());
        string parenttype = insertjson["ArticleType"].get<std::string>();
        string content = insertjson["Content"].get<std::string>();
        int64_t userid = stoll(insertjson["UserId"].get<std::string>());
        string createtime = GetTime();

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];
        bsoncxx::builder::stream::document document{};
        document
            << "_id" << id
            << "ParentId" << parentid
            << "ParentType" << parenttype.data()
            << "Content" << content.data()
            << "UserId" << userid
            << "Likes" << 0
            << "CreateTime" << createtime.data()
            << "Child_Comments" << open_array
            << close_array;

        commentcoll.insert_one(document.view());
        resjson["_id"] = to_string(id);
        resjson["CreateTime"] = createtime.data();
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【插入父评论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：插入子评论
    传入：Json(ParentId,Content,UserId)
    传出：Json(_id,CreateTime)
*/
Json MoDB::InsertSonComment(Json &insertjson)
{
    Json resjson;
    try
    {
        int64_t parentid = stoll(insertjson["ParentId"].get<std::string>().data());
        int64_t id = ++m_commentid;
        string content = insertjson["Content"].get<std::string>();
        int64_t userid = stoll(insertjson["UserId"].get<std::string>().data());
        string createtime = GetTime();

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];

        bsoncxx::builder::stream::document document{};
        document
            << "$addToSet" << open_document
            << "Child_Comments" << open_document
            << "_id" << id
            << "Content"
            << content.data()
            << "UserId" << userid
            << "Likes" << 0
            << "CreateTime"
            << createtime.data()
            << close_document
            << close_document;

        commentcoll.update_one({make_document(kvp("_id", parentid))}, document.view());

        resjson["_id"] = to_string(id);
        resjson["CreateTime"] = createtime;
        resjson["Result"] = "Success";
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【插入子评论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除某一篇文章（讨论，题解，公告）的所有文章，主要服务于删除文章
    传入：Json(ArticleId)
    传出：bool
*/
bool MoDB::DeleteArticleComment(Json &deletejson)
{
    try
    {
        int64_t articleid = stoll(deletejson["ArticleId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];
        auto result = commentcoll.delete_many({make_document(kvp("ParentId", articleid))});

        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "【删除某一篇文章】数据库异常！" ;
        return false;
    }
}

/*
    功能：删除父评论
    传入：Json(CommentId)
    传出：Json(Result,Reason,DeleteNum,ArticleType)
*/
Json MoDB::DeleteFatherComment(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t commentid = stoll(deletejson["CommentId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];

        mongocxx::cursor cursor = commentcoll.find({make_document(kvp("_id", commentid))});
        // 如果未查询到父评论
        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该评论！";
            return resjson;
        }

        mongocxx::pipeline pipe;
        bsoncxx::builder::stream::document document{};

        pipe.match({make_document(kvp("_id", commentid))});
        document
            << "$set" << open_document
            << "Child_Total" << open_document
            << "$size"
            << "$Child_Comments"
            << close_document << close_document;

        pipe.append_stage(document.view());
        cursor = commentcoll.aggregate(pipe);

        Json jsonvalue;
        
        for (auto doc : cursor)
        {
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
        }
        int sonnum = stoi(jsonvalue["Child_Total"].get<std::string>());
        string articletype = jsonvalue["ParentType"].get<std::string>();

        auto result = commentcoll.delete_one({make_document(kvp("_id", commentid))});

        if ((*result).deleted_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到该数据！";
            return resjson;
        }
        resjson["Result"] = "Success";
        resjson["DeleteNum"] = sonnum + 1;
        resjson["ArticleType"] = articletype;
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除父评论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：删除子评论
    传入：Json(CommentId)
    传出：Json(Result,Reason,DeleteNum,ArticleType)
*/
Json MoDB::DeleteSonComment(Json &deletejson)
{
    Json resjson;
    try
    {
        int64_t commentid = stoll(deletejson["CommentId"].get<std::string>());

        auto client = pool.acquire();
        mongocxx::collection commentcoll = (*client)["XDOJ"]["Comment"];
        // 找出父评论ID
        mongocxx::cursor cursor = commentcoll.find({make_document(kvp("Child_Comments._id", commentid))});
        Json jsonvalue;
        

        if (cursor.begin() == cursor.end())
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未查询到数据！";
            return resjson;
        }
        for (auto doc : cursor)
        {
            jsonvalue = Json::parse(bsoncxx::to_json(doc));
        }
        int64_t fatherid = stoll(jsonvalue["_id"].get<std::string>());
        string articletype = jsonvalue["ParentType"].get<std::string>();
        // 删除子评论
        bsoncxx::builder::stream::document document{};
        document
            << "$pull" << open_document
            << "Child_Comments" << open_document
            << "_id" << commentid
            << close_document << close_document;

        auto result = commentcoll.update_one({make_document(kvp("_id", fatherid))}, document.view());

        if ((*result).matched_count() < 1)
        {
            resjson["Result"] = "Fail";
            resjson["Reason"] = "数据库未找到该数据！";
            return resjson;
        }
        resjson["Result"] = "Success";
        resjson["DeleteNum"] = 1;
        resjson["ArticleType"] = articletype;
        return resjson;
    }
    catch (const std::exception &e)
    {
        resjson["Result"] = "500";
        resjson["Reason"] = "数据库异常！";
        LOG_ERROR << "【删除子评论】数据库异常！" ;
        return resjson;
    }
}

/*
    功能：获取某一个集合中最大的ID
*/
int64_t MoDB::GetMaxId(std::string name)
{
    auto client = pool.acquire();
    mongocxx::collection coll = (*client)["XDOJ"][name.data()];

    bsoncxx::builder::stream::document document{};
    mongocxx::pipeline pipe;
    pipe.sort({make_document(kvp("_id", -1))});
    pipe.limit(1);
    mongocxx::cursor cursor = coll.aggregate(pipe);

    // 如果没找到，说明集合中没有数据
    if (cursor.begin() == cursor.end())
        return 0;

    Json jsonvalue;
    
    for (auto doc : cursor)
    {
        jsonvalue = Json::parse(bsoncxx::to_json(doc));
    }

    int64_t id = stoll(jsonvalue["_id"].get<std::string>());
    return id;
}

MoDB::MoDB()
{
    // 初始化ID
    m_problemid = GetMaxId("Problem");
    m_statusrecordid = GetMaxId("StatusRecord");
    m_commentid = GetMaxId("Comment");
    int64_t m_announcementid = GetMaxId("Announcement");
    int64_t m_solutionid = GetMaxId("Solution");
    int64_t m_discussid = GetMaxId("Discuss");
    m_articleid = max(m_solutionid, max(m_discussid, m_announcementid));
}
MoDB::~MoDB()
{
}