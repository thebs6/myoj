#include "StatusRecordList.h"
#include "MongoDataBase.h"
#include "MysqlDataBase.h"
#include "RedisDataBase.h"
#include <iostream>
#include <string>

using namespace std;

StatusRecordList *StatusRecordList::GetInstance()
{
    static StatusRecordList statusrecord;
    return &statusrecord;
}

Json StatusRecordList::SelectStatusRecordList(Json &queryjson)
{
    return MoDB::GetInstance()->SelectStatusRecordList(queryjson);
}

string StatusRecordList::InsertStatusRecord(Json &insertjson)
{
    return MysqlDataBase::GetInstance()->InsertStatusRecord(insertjson);
}

bool StatusRecordList::UpdateStatusRecord(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateStatusRecord(updatejson);
}

Json StatusRecordList::SelectStatusRecord(Json &queryjson)
{
    string statusrecordid = queryjson["SubmitId"].get<std::string>();

    // 获取缓存
    string resstr = ReDB::GetInstance()->GetStatusRecordCache(statusrecordid);

    Json resjson;
    
    // 如果有缓存
    if (resstr != "")
    {
        // 解析缓存json
        resjson = Json::parse(resstr);

        return resjson;
    }

    // 如果没有缓存
    resjson = MoDB::GetInstance()->SelectStatusRecord(queryjson);

    // 添加缓存 （状态不能为等待）
    if (resjson["Result"].get<std::string>() == "Success" && resjson["Status"].get<int>() > 0)
    {
        ReDB::GetInstance()->AddStatusRecordCache(statusrecordid, resjson.dump());
    }

    return resjson;
}

StatusRecordList::StatusRecordList()
{
}

StatusRecordList::~StatusRecordList()
{
}