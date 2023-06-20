#include "OJServer.h"
#include "Control.h"
#include "HttpResponse.h"


Control control;
// 获取请求中的Token
std::string GetRequestToken(const HttpRequest &req)
{
    auto res = req.headers().find("Authorization");

    if (res != req.headers().end())
        return res->second;
    else
        return "0";
}

// 根据返回结果设置状态码
bool SetResponseStatus(const Json &json, HttpResponse*res)
{
    std::string result = json["Result"].get<std::string>();
    if (result == "Success")
    {
        return true;
    }
    else if (result == "Fail")
    {
        return true;
    }
    else if (result == "400") // 请求参数有误
    {
        res->setStatusCode(HttpResponse::k400BadRequest);
    }
    else if (result == "401") // 无权限
    {
        res->setStatusCode(HttpResponse::k401Unauthorized);
    }
    else if (result == "500") // 服务器出错啦
    {
        res->setStatusCode(HttpResponse::k500InternalServerError);
    }
    return true;
}

// 请求注册用户
void doRegisterUser(const HttpRequest &req, HttpResponse*res)
{
    printf("doRegister start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    Json resjson = control.RegisterUser(jsonvalue);

    printf("doGetProblem end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
// 请求登录用户
void doLoginUser(const HttpRequest &req, HttpResponse*res)
{
    printf("doLoginUser start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    Json resjson = control.LoginUser(jsonvalue);

    printf("doLoginUser end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// 通过Token登录用户获取信息
void doGetUserInfoByToken(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetUserInfoByToken start!!!\n");
    Json queryjson;

    queryjson["Token"] = GetRequestToken(req);
    Json resjson = control.LoginUserByToken(queryjson);

    printf("doGetUserInfoByToken end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetUserRank(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetUserRank start!!!\n");
    Json resjson;

    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetUserRank】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectUserRank(queryjson);
    }
    printf("doGetUserRank end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetUserInfo(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetUserInfo start!!!\n");
    Json resjson;

    if (!req.hasParam("UserId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetUserInfo】缺少请求参数！";
    }
    else
    {
        std::string userid = req.getParam("UserId");
        Json queryjson;
        queryjson["UserId"] = userid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectUserInfo(queryjson);
    }

    printf("doGetUserInfo end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
void doUpdateUserInfo(const HttpRequest &req, HttpResponse*res)
{
    printf("doUpdateUserInfo start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.UpdateUserInfo(jsonvalue);
    printf("doUpdateUserInfo end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetUserUpdateInfo(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetUserUpdateInfo start!!!\n");

    Json resjson;

    if (!req.hasParam("UserId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetUserUpdateInfo】缺少请求参数！";
    }
    else
    {
        std::string userid = req.getParam("UserId");
        Json queryjson;
        queryjson["UserId"] = userid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectUserUpdateInfo(queryjson);
    }
    printf("doGetUserUpdateInfo end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
void doGetUserSetInfo(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetUserSetInfo start!!!\n");

    Json resjson;

    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetUserSetInfo】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectUserSetInfo(queryjson);
    }
    printf("doGetUserSetInfo end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doDeleteUser(const HttpRequest &req, HttpResponse*res)
{
    printf("doDeleteUser start!!!\n");

    Json resjson;
    if (!req.hasParam("UserId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doDeleteUser】缺少请求参数！";
    }
    else
    {
        std::string userid = req.getParam("UserId");

        Json jsonvalue = Json::parse(req.body());
        jsonvalue["UserId"] = userid;
        jsonvalue["Token"] = GetRequestToken(req);
        resjson = control.DeleteUser(jsonvalue);
    }

    printf("doDeleteUser end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// 返回网页请求的题目描述
void doGetProblem(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetProblem start!!!\n");
    Json resjson;
    if (!req.hasParam("ProblemId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetProblem】缺少请求参数！";
    }
    else
    {
        std::string problemid = req.getParam("ProblemId");

        Json queryjson;
        queryjson["ProblemId"] = problemid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectProblem(queryjson);
    }
    printf("doGetProblem end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// 返回题库
void doGetProblemList(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetProblemList start!!!\n");

    Json resjson;
    if (!req.hasParam("SearchInfo") || !req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "请求参数有误！";
        LOG_ERROR << "【doGetProblemList】缺少请求参数！";
    }
    else
    {
        Json serachinfo;
        
        // 解析传入的json
        serachinfo = Json::parse(req.getParam("SearchInfo"));
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["SearchInfo"] = serachinfo;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectProblemList(queryjson);
    }
    printf("doGetProblemList end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
// 返回管理员的题库
void doGetProblemListByAdmin(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetProblemListByAdmin start!!!\n");
    Json resjson;
    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "请求参数有误！";
        LOG_ERROR << "【doGetProblemListByAdmin】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");
        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectProblemListByAdmin(queryjson);
    }
    printf("doGetProblemListByAdmin end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// 返回网页请求的题目描述
void doGetProblemInfo(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetProblemInfo start!!!\n");
    Json resjson;

    if (!req.hasParam("ProblemId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "请求参数有误！";
        LOG_ERROR << "【doGetProblemInfo】缺少请求参数！";
    }
    else
    {
        std::string problemid = req.getParam("ProblemId");

        Json queryjson;
        queryjson["ProblemId"] = problemid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectProblemInfoByAdmin(queryjson);
    }
    printf("doGetProblemInfo end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doEditProblem(const HttpRequest &req, HttpResponse*res)
{
    printf("doEditProblem start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["datainfo"]["Token"] = GetRequestToken(req);
    Json resjson = control.EditProblem(jsonvalue["datainfo"]);
    printf("doEditProblem end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doDeleteProblem(const HttpRequest &req, HttpResponse*res)
{
    printf("doDeleteProblem start!!!\n");
    Json resjson;
    if (!req.hasParam("ProblemId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "请求参数有误！";
        LOG_ERROR << "【doDeleteProblem】缺少请求参数！";
    }
    else
    {
        std::string problemid = req.getParam("ProblemId");

        Json deletejson;
        deletejson["ProblemId"] = problemid;
        deletejson["Token"] = GetRequestToken(req);
        resjson = control.DeleteProblem(deletejson);
    }
    printf("doDeleteProblem end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
// 前端提交代码进行判定并返回结果
void doPostCode(const HttpRequest &req, HttpResponse*res)
{
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.GetJudgeCode(jsonvalue);

    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetStatusRecordList(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetStatusRecordList start!!!\n");
    Json resjson;

    if (!req.hasParam("SearchInfo") || !req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetStatusRecordList】缺少请求参数！";
    }
    else
    {
        Json searchinfo;
        
        // 解析传入的json
        searchinfo = Json::parse(req.getParam("SearchInfo"));

        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["SearchInfo"] = searchinfo;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectStatusRecordList(queryjson);
    }
    printf("doGetStatusRecordList end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
void doGetStatusRecord(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetStatusRecordInfo start!!!\n");

    Json resjson;
    if (!req.hasParam("SubmitId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetStatusRecord】缺少请求参数！";
    }
    else
    {
        std::string submitid = req.getParam("SubmitId");

        Json queryjson;
        queryjson["SubmitId"] = submitid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectStatusRecord(queryjson);
    }
    printf("doGetStatusRecordInfo end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
// ------------------公告------------------------------
void doGetAnnouncementList(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetAnnouncementList start!!!\n");

    Json resjson;
    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetAnnouncementList】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectAnnouncementList(queryjson);
    }

    printf("doGetAnnouncementList end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetAnnouncementListByAdmin(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetAnnouncementListByAdmin start!!!\n");

    Json resjson;
    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetAnnouncementListByAdmin】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectAnnouncementListByAdmin(queryjson);
    }

    printf("doGetAnnouncementListByAdmin end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetAnnouncement(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetAnnouncement start!!!\n");
    Json resjson;

    if (!req.hasParam("AnnouncementId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetAnnouncement】缺少请求参数！";
    }
    else
    {
        std::string announcementid = req.getParam("AnnouncementId");
        Json queryjson;
        queryjson["AnnouncementId"] = announcementid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectAnnouncement(queryjson);
    }
    printf("doGetAnnouncement end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doSelectAnnouncement(const HttpRequest &req, HttpResponse*res)
{
    printf("doSelectAnnouncement start!!!\n");

    Json resjson;
    if (!req.hasParam("AnnouncementId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doSelectAnnouncement】缺少请求参数！";
    }
    else
    {
        std::string announcementid = req.getParam("AnnouncementId");

        Json queryjson;
        queryjson["AnnouncementId"] = announcementid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectAnnouncementByEdit(queryjson);
    }
    printf("doSelectAnnouncement end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doInsertAnnouncement(const HttpRequest &req, HttpResponse*res)
{
    printf("doInsertAnnouncement start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.InsertAnnouncement(jsonvalue);
    printf("doInsertAnnouncement end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doUpdateAnnouncement(const HttpRequest &req, HttpResponse*res)
{
    printf("doUpdateAnnouncement start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.UpdateAnnouncement(jsonvalue);
    printf("doUpdateAnnouncement end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doDeleteAnnouncement(const HttpRequest &req, HttpResponse*res)
{
    printf("doDeleteAnnouncement start!!!\n");
    Json resjson;

    if (!req.hasParam("AnnouncementId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doDeleteAnnouncement】缺少请求参数！";
    }
    else
    {
        std::string announcementid = req.getParam("AnnouncementId");
        Json deletejson;
        deletejson["AnnouncementId"] = announcementid;
        deletejson["Token"] = GetRequestToken(req);
        resjson = control.DeleteAnnouncement(deletejson);
    }
    printf("doDeleteAnnouncement end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// ------------------题解------------------------------
void doGetSolutionList(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetSolutionList start!!!\n");
    Json resjson;

    if (!req.hasParam("SearchInfo") || !req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetSolutionList】缺少请求参数！";
    }
    else
    {
        Json searchinfo;
        
        // 解析传入的json
        searchinfo = Json::parse(req.getParam("SearchInfo"));
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");
        Json queryjson;
        queryjson["SearchInfo"] = searchinfo;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectSolutionList(queryjson);
    }
    printf("doGetSolutionList end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetSolutionListByAdmin(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetSolutionListByAdmin start!!!\n");
    Json resjson;
    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetSolutionListByAdmin】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectSolutionListByAdmin(queryjson);
    }
    printf("doGetSolutionListByAdmin end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
void doGetSolution(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetSolution start!!!\n");
    Json resjson;

    if (!req.hasParam("SolutionId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetSolution】缺少请求参数！";
    }
    else
    {
        std::string solutionid = req.getParam("SolutionId");
        Json queryjson;
        queryjson["SolutionId"] = solutionid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectSolution(queryjson);
    }
    printf("doGetSolution end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doSelectSolutionByEdit(const HttpRequest &req, HttpResponse*res)
{
    printf("doSelectSolutionByEdit start!!!\n");
    Json resjson;

    if (!req.hasParam("SolutionId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doSelectSolutionByEdit】缺少请求参数！";
    }
    else
    {
        std::string solutionid = req.getParam("SolutionId");
        Json queryjson;
        queryjson["SolutionId"] = solutionid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectSolutionByEdit(queryjson);
    }
    printf("doSelectSolutionByEdit end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doInsertSolution(const HttpRequest &req, HttpResponse*res)
{
    printf("doInsertSolution start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.InsertSolution(jsonvalue);
    printf("doInsertSolution end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doUpdateSolution(const HttpRequest &req, HttpResponse*res)
{
    printf("doUpdateSolution start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.UpdateSolution(jsonvalue);
    printf("doUpdateSolution end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doDeleteSolution(const HttpRequest &req, HttpResponse*res)
{
    printf("doDeleteSolution start!!!\n");
    Json resjson;

    if (!req.hasParam("SolutionId") || !req.hasParam("UserId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doDeleteSolution】缺少请求参数！";
    }
    else
    {
        std::string solutionid = req.getParam("SolutionId");
        std::string userid = req.getParam("UserId");
        Json deletejson;
        deletejson["SolutionId"] = solutionid;
        deletejson["UserId"] = userid;
        deletejson["Token"] = GetRequestToken(req);
        resjson = control.DeleteSolution(deletejson);
    }
    printf("doDeleteSolution end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// ------------------讨论------------------------------
void doGetDiscussList(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetDiscussList start!!!\n");
    Json resjson;
    if (!req.hasParam("SearchInfo") || !req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetDiscussList】缺少请求参数！";
    }
    else
    {
        Json searchinfo;
        
        // 解析传入的json
        searchinfo = Json::parse(req.getParam("SearchInfo"));
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");
        Json queryjson;
        queryjson["SearchInfo"] = searchinfo;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectDiscussList(queryjson);
    }
    printf("doGetDiscussList end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetDiscussListByAdmin(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetDiscussListByAdmin start!!!\n");
    Json resjson;
    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetDiscussListByAdmin】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");
        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectDiscussListByAdmin(queryjson);
    }
    printf("doGetDiscussListByAdmin end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
void doGetDiscuss(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetDiscuss start!!!\n");
    Json resjson;
    if (!req.hasParam("DiscussId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetDiscuss】缺少请求参数！";
    }
    else
    {
        std::string discussid = req.getParam("DiscussId");
        Json queryjson;
        queryjson["DiscussId"] = discussid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectDiscuss(queryjson);
    }
    printf("doGetDiscuss end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doSelectDiscussByEdit(const HttpRequest &req, HttpResponse*res)
{
    printf("doSelectDiscussByEdit start!!!\n");
    Json resjson;
    if (!req.hasParam("DiscussId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doSelectDiscussByEdit】缺少请求参数！";
    }
    else
    {
        std::string discussid = req.getParam("DiscussId");
        Json queryjson;
        queryjson["DiscussId"] = discussid;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectDiscussByEdit(queryjson);
    }
    printf("doSelectDiscussByEdit end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doInsertDiscuss(const HttpRequest &req, HttpResponse*res)
{
    printf("doInsertSolution start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.InsertDiscuss(jsonvalue);
    printf("doInsertSolution end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doUpdateDiscuss(const HttpRequest &req, HttpResponse*res)
{
    printf("doUpdateSolution start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Token"] = GetRequestToken(req);
    Json resjson = control.UpdateDiscuss(jsonvalue);
    printf("doUpdateSolution end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doDeleteDiscuss(const HttpRequest &req, HttpResponse*res)
{
    printf("doDeleteDiscuss start!!!\n");
    Json resjson;

    if (!req.hasParam("DiscussId") || !req.hasParam("UserId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doDeleteDiscuss】缺少请求参数！";
    }
    else
    {
        std::string discussid = req.getParam("DiscussId");
        std::string userid = req.getParam("UserId");

        Json deletejson;
        deletejson["DiscussId"] = discussid;
        deletejson["UserId"] = userid;
        deletejson["Token"] = GetRequestToken(req);
        resjson = control.DeleteDiscuss(deletejson);
    }
    printf("doDeleteDiscuss end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doSelectCommentListByAdmin(const HttpRequest &req, HttpResponse*res)
{
    printf("doSelectCommentListByAdmin start!!!\n");
    Json resjson;

    if (!req.hasParam("Page") || !req.hasParam("PageSize"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doSelectCommentListByAdmin】缺少请求参数！";
    }
    else
    {
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");

        Json queryjson;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.SelectCommentListByAdmin(queryjson);
    }
    printf("doSelectCommentListByAdmin end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doGetComment(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetComment start!!!\n");
    Json resjson;

    if (!req.hasParam("Type") || !req.hasParam("ParentId") || !req.hasParam("Page") || !req.hasParam("PageSize") || !req.hasParam("SonNum"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doGetComment】缺少请求参数！";
    }
    else
    {
        std::string type = req.getParam("Type");
        std::string parentid = req.getParam("ParentId");
        std::string page = req.getParam("Page");
        std::string pagesize = req.getParam("PageSize");
        std::string sonsnum = req.getParam("SonNum");
        Json queryjson;
        queryjson["Type"] = type;
        queryjson["ParentId"] = parentid;
        queryjson["Page"] = page;
        queryjson["PageSize"] = pagesize;
        queryjson["SonNum"] = sonsnum;
        queryjson["Token"] = GetRequestToken(req);
        resjson = control.GetComment(queryjson);
    }

    printf("doGetComment end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

void doInsertComment(const HttpRequest &req, HttpResponse*res)
{
    printf("doInsertComment start!!!\n");
    Json jsonvalue = Json::parse(req.body());
    
    // 解析传入的json
    
    jsonvalue["Info"]["Token"] = GetRequestToken(req);
    Json resjson = control.InsertComment(jsonvalue["Info"]);
    printf("doInsertComment end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}
void doDeleteComment(const HttpRequest &req, HttpResponse*res)
{
    printf("doDeleteComment start!!!\n");
    Json resjson;
    if (!req.hasParam("ArticleId") || !req.hasParam("CommentId"))
    {
        resjson["Result"] = "400";
        resjson["Reason"] = "缺少请求参数！";
        LOG_ERROR << "【doDeleteComment】缺少请求参数！";
    }
    else
    {
        std::string articleid = req.getParam("ArticleId");
        std::string commentid = req.getParam("CommentId");
        Json deletejson;
        deletejson["ArticleId"] = articleid;
        deletejson["CommentId"] = commentid;
        deletejson["Token"] = GetRequestToken(req);
        resjson = control.DeleteComment(deletejson);
    }
    printf("doDeleteComment end!!!\n");
    SetResponseStatus(resjson, res);
    res->setBody(resjson.dump());
}

// void doGetImage(const HttpRequest &req, HttpResponse*res)
// {
//     printf("doGetImage start!!!\n");
//     int index = stoi(req.matches[1]);
//     std::string path = "../../WWW/image/avatar" + to_string(index) + ".webp";
//     ifstream infile;
//     infile.open(path.data());
//     if (!infile.is_open())
//     {
//         std::string str = "图片获取失败";
//         res.set_content(str, "text");
//     }
//     std::string image((istreambuf_iterator<char>(infile)),
//                  (istreambuf_iterator<char>()));
//     printf("doGetImage end!!!\n");
//     res.set_content(image, "webp");
// }

void doGetTags(const HttpRequest &req, HttpResponse*res)
{
    printf("doGetTags start!!!\n");
    Json queryjson;
    std::string tagtype = req.getParam("TagType");
    queryjson["TagType"] = tagtype;

    Json resjson = control.GetTags(queryjson);
    printf("doGetTags end!!!\n");
    res->setBody(resjson.dump());
}

void OJServer::onRequest(const HttpRequest& req, HttpResponse* resp) {
    if(req.getMethod() == HttpRequest::kGet && get_map_.find(req.path()) != get_map_.end()) {
        get_map_[req.path()](req, resp);
    } else if(req.getMethod() == HttpRequest::kPost && post_map_.find(req.path()) != post_map_.end()) {
            post_map_[req.path()](req, resp);
    }

    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("application/json");

}

void OJServer::run() {

    Logger::setLogLevel(Logger::LogLevel::DEBUG);
    server_.setHttpCallback(std::move(std::bind(&OJServer::onRequest, this, std::placeholders::_1, std::placeholders::_2)));


    // -----------------用户----------------
    // 注册用户
    Post("/user/register", doRegisterUser);
    // 登录用户
    Post("/user/login", doLoginUser);
    // 返回用户信息 Token登录
    Get("/user/tokenlogin", doGetUserInfoByToken);
    // 返回用户排名
    Get("/user/rank", doGetUserRank);
    // 返回用户信息，用于主页展示
    Get("/user/home", doGetUserInfo);
    // 更新用户信息
    Post("/user/update", doUpdateUserInfo);
    // 返回用户信息，用于编辑修改
    Get("/user/updateinfo", doGetUserUpdateInfo);
    // 分页获取用户信息
    Get("/userset", doGetUserSetInfo);
    // 更新用户信息
    Post("/user", doDeleteUser);

    // ----------------题目------------------
    // 获取单个题目
    Get("/problem", doGetProblem);
    // 获取题库
    Get("/problemlist", doGetProblemList);
    // 管理员获取题库
    Get("/problemlist/admin", doGetProblemListByAdmin);
    // 获取单个题目详细信息
    Get("/problem/select", doGetProblemInfo);
    // 编辑题目 包含添加题目，修改题目
    Post("/problem/edit", doEditProblem);
    // 删除题目
    Post("/problem", doDeleteProblem);

    // ---------------测评-----------------
    // 获取状态记录
    Get("/statusrecordlist", doGetStatusRecordList);
    // 获取一条测评记录
    Get("/statusrecord", doGetStatusRecord);

    // 提交代码
    Post("/problemcode", doPostCode);

    // --------------公告--------------------
    // 获取公告列表
    Get("/announcementlist", doGetAnnouncementList);

    // 获取公告列表
    Get("/announcementlist/admin", doGetAnnouncementListByAdmin);

    // 获取公告内容
    Get("/announcement", doGetAnnouncement);

    // 获取公告信息用于编辑
    Get("/announcement/select", doSelectAnnouncement);

    // 用户提交公告
    Post("/announcement/insert", doInsertAnnouncement);

    // 用户修改公告
    Post("/announcement/update", doUpdateAnnouncement);

    // 用户删除公告
    Post("/announcement", doDeleteAnnouncement);

    // -------------题解------------------------
    // 获取题解列表
    Get("/solutionlist", doGetSolutionList);

    // 获取题解列表
    Get("/solutionlist/admin", doGetSolutionListByAdmin);

    // 获取题解内容
    Get("/solution", doGetSolution);

    // 获取题解信息用于编辑
    Get("/solution/select", doSelectSolutionByEdit);

    // 用户题解公告
    Post("/solution/insert", doInsertSolution);

    // 用户修改题解
    Post("/solution/update", doUpdateSolution);

    // 用户删除题解
    Post("/solution", doDeleteSolution);

    // -------------讨论------------------------
    // 获取讨论列表
    Get("/discusslist", doGetDiscussList);

    // 管理员获取讨论列表
    Get("/discusslist/admin", doGetDiscussListByAdmin);

    // 获取讨论内容
    Get("/discuss", doGetDiscuss);

    // 获取讨论信息用于编辑
    Get("/discuss/select", doSelectDiscussByEdit);

    // 用户提交讨论
    Post("/discuss/insert", doInsertDiscuss);

    // 用户修改讨论
    Post("/discuss/update", doUpdateDiscuss);

    // 用户删除讨论
    Post("/discuss", doDeleteDiscuss);

    // ---------------评论--------------------
    // 获取评论
    Get("/commentlist/admin", doSelectCommentListByAdmin);
    // 获取评论
    Get("/comment", doGetComment);
    // 提交评论
    Post("/comment/insert", doInsertComment);
    // 删除评论
    Post("/comment", doDeleteComment);

    // 获取图片资源
    // Get(R"(/image/(\d+))", doGetImage);
    // 获取标签
    // Get("/tags", doGetTags);
    // // 设置静态资源
    // set_base_dir("../WWW");


    server_.start();
}