#include "CommentList.h"
#include "MongoDataBase.h"

CommentList *CommentList::GetInstance()
{
    static CommentList commentlist;
    return &commentlist;
}

Json CommentList::SelectCommentListByAdmin(Json &queryjson)
{
    return MoDB::GetInstance()->SelectCommentListByAdmin(queryjson);
}

Json CommentList::getFatherComment(Json &queryjson)
{
    return MoDB::GetInstance()->getFatherComment(queryjson);
}
Json CommentList::getSonComment(Json &queryjson)
{
    return MoDB::GetInstance()->getSonComment(queryjson);
}

// 插入父评论
Json CommentList::InsertFatherComment(Json &insertjson)
{
    return MoDB::GetInstance()->InsertFatherComment(insertjson);
}

// 插入子评论
Json CommentList::InsertSonComment(Json &insertjson)
{
    return MoDB::GetInstance()->InsertSonComment(insertjson);
}

bool CommentList::DeleteArticleComment(Json &deletejson)
{
    return MoDB::GetInstance()->DeleteArticleComment(deletejson);
}

Json CommentList::DeleteFatherComment(Json &deletejson)
{
    return MoDB::GetInstance()->DeleteFatherComment(deletejson);
}

Json CommentList::DeleteSonComment(Json &deletejson)
{
    return MoDB::GetInstance()->DeleteSonComment(deletejson);
}

CommentList::CommentList()
{
}
CommentList::~CommentList()
{
}