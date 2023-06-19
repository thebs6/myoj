#ifndef ANNOUNCEMENTLIST_H
#define ANNOUNCEMENTLIST_H
#include "json.hpp"
using Json = nlohmann::json;
#include <string>
// 公告类
class AnnouncementList
{
public:
    // 局部静态特性的方式实现单实例
    static AnnouncementList *GetInstance();

    // 添加公告
    Json InsertAnnouncement(Json &insertjson);

    // 分页查询公告
    Json SelectAnnouncementList(Json &queryjson);

    // 分页查询公告
    Json SelectAnnouncementListByAdmin(Json &queryjson);

    // 查询
    Json SelectAnnouncementByEdit(Json &queryjson);

    // 查询公告的内容
    Json SelectAnnouncement(Json &queryjson);

    // 修改评论数的数量
    bool UpdateAnnouncementComments(Json &updatejson);

    // 更新公告
    Json UpdateAnnouncement(Json &updatejson);

    // 删除公告
    Json DeleteAnnouncement(Json &deletejson);

private:
    AnnouncementList();
    ~AnnouncementList();
};
#endif