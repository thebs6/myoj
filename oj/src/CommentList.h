#ifndef COMMENTLIST_H
#define COMMENTLIST_H
#include "json.hpp"
using Json = nlohmann::json;
#include <string>
// 讨论类
class CommentList
{
public:
    // 局部静态特性的方式实现单实例
    static CommentList *GetInstance();

    // 管理员查询评论
    Json SelectCommentListByAdmin(Json &queryjson);

    // 获取父评论
    Json getFatherComment(Json &queryjson);

    // 获取子评论
    Json getSonComment(Json &queryjson);

    // 插入父评论
    Json InsertFatherComment(Json &insertjson);

    // 插入子评论
    Json InsertSonComment(Json &insertjson);

    // 删除某一篇文章的所有评论
    bool DeleteArticleComment(Json &deletejson);

    // 删除父评论
    Json DeleteFatherComment(Json &deletejson);

    // 删除子评论
    Json DeleteSonComment(Json &deletejson);

private:
    CommentList();
    ~CommentList();
};
#endif