#ifndef STATUSRECORDLIST_H
#define STATUSRECORDLIST_H
#include "json.hpp"
using Json = nlohmann::json;
#include <string>
// 状态记录类，保存代码判定结果的类
class StatusRecordList
{
public:
    // 局部静态特性的方式实现单实例
    static StatusRecordList *GetInstance();

    // 分页查询测评记录
    Json SelectStatusRecordList(Json &queryjson);

    // 插入查询记录
    std::string InsertStatusRecord(Json &insertjson);

    // 更新测评信息返回测评结果
    bool UpdateStatusRecord(Json &updatejson);

    // 查询一条详细测评记录
    Json SelectStatusRecord(Json &queryjson);

private:
    StatusRecordList();
    ~StatusRecordList();
};

#endif