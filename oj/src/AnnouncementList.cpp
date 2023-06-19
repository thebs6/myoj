#include "AnnouncementList.h"
#include "MongoDataBase.h"
// 局部静态特性的方式实现单实例
AnnouncementList *AnnouncementList::GetInstance()
{
    static AnnouncementList announcementlist;
    return &announcementlist;
}

Json AnnouncementList::InsertAnnouncement(Json &insertjson)
{
    return MoDB::GetInstance()->InsertAnnouncement(insertjson);
}

Json AnnouncementList::SelectAnnouncementList(Json &queryjson)
{
    return MoDB::GetInstance()->SelectAnnouncementList(queryjson);
}

Json AnnouncementList::SelectAnnouncementListByAdmin(Json &queryjson)
{
    return MoDB::GetInstance()->SelectAnnouncementList(queryjson);
}

Json AnnouncementList::SelectAnnouncementByEdit(Json &queryjson)
{
    return MoDB::GetInstance()->SelectAnnouncementByEdit(queryjson);
}

Json AnnouncementList::SelectAnnouncement(Json &queryjson)
{
    return MoDB::GetInstance()->SelectAnnouncement(queryjson);
}

bool AnnouncementList::UpdateAnnouncementComments(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateAnnouncementComments(updatejson);
}

Json AnnouncementList::UpdateAnnouncement(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateAnnouncement(updatejson);
}

Json AnnouncementList::DeleteAnnouncement(Json &deletejson)
{
    return MoDB::GetInstance()->DeleteAnnouncement(deletejson);
}

AnnouncementList::AnnouncementList()
{
}
AnnouncementList::~AnnouncementList()
{
}