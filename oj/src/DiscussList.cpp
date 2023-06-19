#include "DiscussList.h"
#include "MongoDataBase.h"

DiscussList *DiscussList::GetInstance()
{
    static DiscussList disscusslist;
    return &disscusslist;
}

Json DiscussList::InsertDiscuss(Json &insertjson)
{
    return MoDB::GetInstance()->InsertDiscuss(insertjson);
}

Json DiscussList::SelectDiscussList(Json &queryjson)
{
    return MoDB::GetInstance()->SelectDiscussList(queryjson);
}

Json DiscussList::SelectDiscussListByAdmin(Json &queryjson)
{
    return MoDB::GetInstance()->SelectDiscussListByAdmin(queryjson);
}

Json DiscussList::SelectDiscussByEdit(Json &queryjson)
{
    return MoDB::GetInstance()->SelectDiscussByEdit(queryjson);
}

Json DiscussList::SelectDiscuss(Json &queryjson)
{
    return MoDB::GetInstance()->SelectDiscuss(queryjson);
}

bool DiscussList::UpdateDiscussComments(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateDiscussComments(updatejson);
}

Json DiscussList::UpdateDiscuss(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateDiscuss(updatejson);
}

Json DiscussList::DeleteDiscuss(Json &deletejson)
{
    return MoDB::GetInstance()->DeleteDiscuss(deletejson);
}
DiscussList::DiscussList()
{
}
DiscussList::~DiscussList()
{
}