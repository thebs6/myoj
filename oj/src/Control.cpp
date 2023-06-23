#include "Control.h"
#include "ProblemList.h"
#include "UserList.h"
#include "StatusRecordList.h"
#include "DiscussList.h"
#include "CommentList.h"
#include "SolutionList.h"
#include "AnnouncementList.h"
#include "Tag.h"
#include "Judger.h"
#include <iostream>
using namespace std;

// 无权限
static Json NoPermission;

Json Control::RegisterUser(Json &registerjson)
{
	return UserList::GetInstance()->RegisterUser(registerjson);
}

Json Control::LoginUser(Json &loginjson)
{
	return UserList::GetInstance()->LoginUser(loginjson);
}

Json Control::LoginUserByToken(Json &loginjson)
{
	return UserList::GetInstance()->LoginUserByToken(loginjson);
}

Json Control::SelectUserRank(Json &queryjson)
{
	return UserList::GetInstance()->SelectUserRank(queryjson);
}

Json Control::SelectUserInfo(Json &queryjson)
{
	return UserList::GetInstance()->SelectUserInfo(queryjson);
}

Json Control::UpdateUserInfo(Json &updatejson)
{
	// 如果不是本人或者管理员 无权修改
	if (!UserList::GetInstance()->IsAuthor(updatejson))
		return NoPermission;

	return UserList::GetInstance()->UpdateUserInfo(updatejson);
}

Json Control::SelectUserUpdateInfo(Json &queryjson)
{
	// 如果不是本人或者管理员 无权查询
	if (!UserList::GetInstance()->IsAuthor(queryjson))
		return NoPermission;

	return UserList::GetInstance()->SelectUserUpdateInfo(queryjson);
}

Json Control::SelectUserSetInfo(Json &queryjson)
{
	// 如果不是管理员，无权查询
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;

	return UserList::GetInstance()->SelectUserSetInfo(queryjson);
}

Json Control::DeleteUser(Json &deletejson)
{
	// 如果不是管理员，无权删除
	if (!UserList::GetInstance()->IsAdministrator(deletejson))
		return NoPermission;

	return UserList::GetInstance()->DeleteUser(deletejson);
}

Json Control::SelectProblemInfoByAdmin(Json &queryjson)
{
	// 如果不是管理员，无权查看
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;

	return ProblemList::GetInstance()->SelectProblemInfoByAdmin(queryjson);
}

Json Control::SelectProblem(Json &queryjson)
{
	return ProblemList::GetInstance()->SelectProblem(queryjson);
}

Json Control::EditProblem(Json &insertjson)
{
	// 如果不是管理员，无权操作
	if (!UserList::GetInstance()->IsAdministrator(insertjson))
		return NoPermission;

	Json resjson;
	if (insertjson["EditType"].get<std::string>() == "Insert")
	{
		resjson = ProblemList::GetInstance()->InsertProblem(insertjson);
	}
	else if (insertjson["EditType"].get<std::string>() == "Update")
	{
		resjson = ProblemList::GetInstance()->UpdateProblem(insertjson);
	}
	Tag::GetInstance()->InitProblemTags();
	return resjson;
}
Json Control::DeleteProblem(Json &deletejson)
{
	// 如果不是管理员，无权操作
	if (!UserList::GetInstance()->IsAdministrator(deletejson))
		return NoPermission;

	return ProblemList::GetInstance()->DeleteProblem(deletejson);
}

Json Control::SelectProblemList(Json &queryjson)
{
	return ProblemList::GetInstance()->SelectProblemList(queryjson);
}

Json Control::SelectProblemListByAdmin(Json &queryjson)
{
	// 如果不是管理员，无权查询
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;

	return ProblemList::GetInstance()->SelectProblemListByAdmin(queryjson);
}

Json Control::GetJudgeCode(Json judgejson)
{
	//如果不是用户，无权操作
	// if (!UserList::GetInstance()->IsOrdinaryUser(judgejson))
	// 	return NoPermission;

	Json resjson;
	// 传入Json(ProblemId,UserId,UserNickName,Code,Language,TimeLimit,MemoryLimit,JudgeNum,ProblemTitle)

	// 添加状态记录
	// 传入：Json(ProblemId,UserId,UserNickName,ProblemTitle,Language,Code);
	// Json insertjson;
	

	// if (submitid == "0")
	// {
	// 	resjson["Result"] = "Fail";
	// 	resjson["Reason"] = "系统出错！";
	// 	return resjson;
	// }

	// 运行代码
	Json runjson;

	runjson["UserId"] = judgejson["UserId"];
	runjson["UserNickName"] = judgejson["UserNickName"];
	runjson["ProblemTitle"] = judgejson["ProblemTitle"];

	runjson["Code"] = judgejson["Code"];
	runjson["SubmitId"] = judgejson["SubmitId"];
	runjson["ProblemId"] = judgejson["ProblemId"];
	runjson["Language"] = judgejson["Language"];
	runjson["JudgeNum"] = judgejson["JudgeNum"];
	runjson["TimeLimit"] = judgejson["TimeLimit"];
	runjson["MemoryLimit"] = judgejson["MemoryLimit"];

	// 创建判题对象
	Judger judger;
	Json json = judger.Run(runjson);

	// 更新状态信息
	/*
		传入：Json(SubmitId,Status,RunTime,RunMemory,Length,ComplierInfo,
		TestInfo[(Status,StandardOutput,PersonalOutput,RunTime,RunMemory)])
	*/
	string submitid = StatusRecordList::GetInstance()->InsertStatusRecord(json);

	// 更新题目的状态
	Json updatejson;
	updatejson["ProblemId"] = judgejson["ProblemId"];
	updatejson["Status"] = json["Status"];
	ProblemList::GetInstance()->UpdateProblemStatusNum(updatejson);

	updatejson["UserId"] = judgejson["UserId"];

	// 更新用户的状态
	if (UserList::GetInstance()->UpdateUserProblemInfo(updatejson))
		resjson["IsFirstAC"] = true;
	else
		resjson["IsFirstAC"] = false;

	resjson["Result"] = "Success";
	resjson["Status"] = json["Status"];
	resjson["ComplierInfo"] = json["ComplierInfo"];
	return resjson;
}

Json Control::SelectStatusRecordList(Json &queryjson)
{
	return StatusRecordList::GetInstance()->SelectStatusRecordList(queryjson);
}

Json Control::SelectStatusRecord(Json &queryjson)
{
	return StatusRecordList::GetInstance()->SelectStatusRecord(queryjson);
}

Json Control::SelectCommentListByAdmin(Json &queryjson)
{
	// 如果不是管理员，无权查看
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;

	return CommentList::GetInstance()->SelectCommentListByAdmin(queryjson);
}

Json Control::GetComment(Json &queryjson)
{
	if (queryjson["Type"].get<std::string>() == "Father")
	{
		return CommentList::GetInstance()->getFatherComment(queryjson);
	}
	else
	{
		return CommentList::GetInstance()->getSonComment(queryjson);
	}
}

Json Control::InsertComment(Json &insertjson)
{
	if (!UserList::GetInstance()->IsOrdinaryUser(insertjson))
		return NoPermission;
	// 文章添加评论数
	Json updatejson;
	updatejson["ArticleId"] = insertjson["ArticleId"];
	updatejson["Num"] = 1;
	if (insertjson["ArticleType"].get<std::string>() == "Discuss")
	{
		DiscussList::GetInstance()->UpdateDiscussComments(updatejson);
	}
	else if (insertjson["ArticleType"].get<std::string>() == "Solution")
	{
		SolutionList::GetInstance()->UpdateSolutionComments(updatejson);
	}
	else if (insertjson["ArticleType"].get<std::string>() == "Announcement")
	{
		AnnouncementList::GetInstance()->UpdateAnnouncementComments(updatejson);
	}

	if (insertjson["Type"].get<std::string>() == "Father") // 父评论
	{

		return CommentList::GetInstance()->InsertFatherComment(insertjson);
	}
	else // 子评论
	{
		return CommentList::GetInstance()->InsertSonComment(insertjson);
	}
}

// Json(ArticleId,CommentId)
Json Control::DeleteComment(Json &deletejson)
{
	// 如果不是用户，没有权限删除
	if (!UserList::GetInstance()->IsOrdinaryUser(deletejson))
		return NoPermission;

	string articleid = deletejson["ArticleId"].get<std::string>();

	Json resjson;
	// 删除父评论
	Json json = CommentList::GetInstance()->DeleteFatherComment(deletejson);
	// 如果失败删除子评论
	if (json["Result"] == "Fail")
		json = CommentList::GetInstance()->DeleteSonComment(deletejson);
	// 如果都失败，返回失败结果
	if (json["Result"] == "Fail")
	{
		resjson["Result"] = "Fail";
		resjson["Reason"] = "数据库未查询到数据！";
		return resjson;
	}
	// 如果删除评论成功，更新文章的评论数量
	Json articlejson;
	articlejson["Num"] = stoi(json["DeleteNum"].get<std::string>()) * -1;
	articlejson["ArticleId"] = articleid;
	string articletype = json["ArticleType"].get<std::string>();
	if (articletype == "Discuss")
	{
		DiscussList::GetInstance()->UpdateDiscussComments(articlejson);
	}
	else if (articletype == "Solution")
	{
		SolutionList::GetInstance()->UpdateSolutionComments(articlejson);
	}
	else if (articletype == "Announcement")
	{
		AnnouncementList::GetInstance()->UpdateAnnouncementComments(articlejson);
	}

	resjson["Result"] = "Success";
	return resjson;
}
// --------------------公告------------------------
Json Control::SelectAnnouncementList(Json &queryjson)
{
	return AnnouncementList::GetInstance()->SelectAnnouncementList(queryjson);
}

Json Control::SelectAnnouncementListByAdmin(Json &queryjson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;
	return AnnouncementList::GetInstance()->SelectAnnouncementListByAdmin(queryjson);
}

Json Control::SelectAnnouncement(Json &queryjson)
{
	return AnnouncementList::GetInstance()->SelectAnnouncement(queryjson);
}

Json Control::SelectAnnouncementByEdit(Json &queryjson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;
	return AnnouncementList::GetInstance()->SelectAnnouncementByEdit(queryjson);
}

Json Control::InsertAnnouncement(Json &insertjson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(insertjson))
		return NoPermission;

	return AnnouncementList::GetInstance()->InsertAnnouncement(insertjson);
}

Json Control::UpdateAnnouncement(Json &updatejson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(updatejson))
		return NoPermission;

	return AnnouncementList::GetInstance()->UpdateAnnouncement(updatejson);
}
Json Control::DeleteAnnouncement(Json &deletejson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(deletejson))
		return NoPermission;

	Json resjson = AnnouncementList::GetInstance()->DeleteAnnouncement(deletejson);
	if (resjson["Result"].get<std::string>() == "Success")
	{
		Json json;
		json["ArticleId"] = deletejson["AnnouncementId"];
		CommentList::GetInstance()->DeleteArticleComment(json);
	}
	return resjson;
}

// ----------------------题解----------------------------
Json Control::SelectSolutionList(Json &queryjson)
{
	return SolutionList::GetInstance()->SelectSolutionList(queryjson);
}

Json Control::SelectSolutionListByAdmin(Json &queryjson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;

	return SolutionList::GetInstance()->SelectSolutionListByAdmin(queryjson);
}

Json Control::SelectSolution(Json &queryjson)
{
	return SolutionList::GetInstance()->SelectSolution(queryjson);
}

Json Control::SelectSolutionByEdit(Json &queryjson)
{
	// 如果不是用户，无权限
	if (!UserList::GetInstance()->IsOrdinaryUser(queryjson))
		return NoPermission;

	return SolutionList::GetInstance()->SelectSolutionByEdit(queryjson);
}

Json Control::InsertSolution(Json &insertjson)
{
	// 如果不是用户，无权限
	if (!UserList::GetInstance()->IsOrdinaryUser(insertjson))
		return NoPermission;

	return SolutionList::GetInstance()->InsertSolution(insertjson);
}

Json Control::UpdateSolution(Json &updatejson)
{
	// 如果不是作者或者管理员，无权限
	if (!UserList::GetInstance()->IsAuthor(updatejson))
		return NoPermission;

	return SolutionList::GetInstance()->UpdateSolution(updatejson);
}
Json Control::DeleteSolution(Json &deletejson)
{
	// 如果不是作者或者管理员，无权限
	if (!UserList::GetInstance()->IsAuthor(deletejson))
		return NoPermission;

	Json resjson = SolutionList::GetInstance()->DeleteSolution(deletejson);
	if (resjson["Result"].get<std::string>() == "Success")
	{
		Json json;
		json["ArticleId"] = deletejson["SolutionId"];
		CommentList::GetInstance()->DeleteArticleComment(json);
	}
	return resjson;
}

// ----------------------讨论----------------------------
Json Control::SelectDiscussList(Json &queryjson)
{
	return DiscussList::GetInstance()->SelectDiscussList(queryjson);
}

Json Control::SelectDiscussListByAdmin(Json &queryjson)
{
	// 如果不是管理员，无权限
	if (!UserList::GetInstance()->IsAdministrator(queryjson))
		return NoPermission;

	return DiscussList::GetInstance()->SelectDiscussListByAdmin(queryjson);
}

Json Control::SelectDiscuss(Json &queryjson)
{
	return DiscussList::GetInstance()->SelectDiscuss(queryjson);
}

Json Control::SelectDiscussByEdit(Json &queryjson)
{
	// 如果不是用户，无权限
	if (!UserList::GetInstance()->IsOrdinaryUser(queryjson))
		return NoPermission;

	return DiscussList::GetInstance()->SelectDiscussByEdit(queryjson);
}

Json Control::InsertDiscuss(Json &insertjson)
{
	// 如果不是用户，无权限
	if (!UserList::GetInstance()->IsOrdinaryUser(insertjson))
		return NoPermission;

	return DiscussList::GetInstance()->InsertDiscuss(insertjson);
}

Json Control::UpdateDiscuss(Json &updatejson)
{
	// 如果不是作者或者管理员，无权限
	if (!UserList::GetInstance()->IsAuthor(updatejson))
		return NoPermission;

	return DiscussList::GetInstance()->UpdateDiscuss(updatejson);
}
Json Control::DeleteDiscuss(Json &deletejson)
{
	// 如果不是作者或者管理员，无权限
	if (!UserList::GetInstance()->IsAuthor(deletejson))
		return NoPermission;

	Json resjson = DiscussList::GetInstance()->DeleteDiscuss(deletejson);
	if (resjson["Result"].get<std::string>() == "Success")
	{
		Json json;
		json["ArticleId"] = deletejson["DiscussId"];
		CommentList::GetInstance()->DeleteArticleComment(json);
	}
	return resjson;
}

Json Control::GetTags(Json &queryjson)
{
	if (queryjson["TagType"].get<std::string>() == "Problem")
	{
		return Tag::GetInstance()->getProblemTags();
	}
	return Json();
}

Control::Control()
{
	// // 初始化题目标签
	// Tag::GetInstance()->InitProblemTags();

	// // 初始化用户权限
	// UserList::GetInstance()->InitUserAuthority();

	// 初始化返回变量
	NoPermission["Result"] = "401";
	NoPermission["Reason"] = "无权限";
}

Control::~Control()
{
}