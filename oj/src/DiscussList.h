#ifndef DISCUSSLIST_H
#define DISCUSSLIST_H
#include "json.hpp"
using Json = nlohmann::json;
#include <string>
// 讨论类
class DiscussList
{
public:
    // 局部静态特性的方式实现单实例
    static DiscussList *GetInstance();

    // 添加讨论
    Json InsertDiscuss(Json &insertjson);

    // 分页查询讨论
    Json SelectDiscussList(Json &queryjson);

    // 管理员分页查询
    Json SelectDiscussListByAdmin(Json &queryjson);

    // 查询
    Json SelectDiscussByEdit(Json &queryjson);

    // 查询讨论的内容
    Json SelectDiscuss(Json &queryjson);

    // 修改评论数的数量
    bool UpdateDiscussComments(Json &updatejson);

    // 更新讨论
    Json UpdateDiscuss(Json &updatejson);

    // 删除讨论
    Json DeleteDiscuss(Json &deletejson);

private:
    DiscussList();
    ~DiscussList();
};
#endif