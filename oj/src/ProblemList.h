#ifndef PROBLEMLIST_H
#define PROBLEMLIST_H

#include <map>
#include <string>
#include "json.hpp"
using Json = nlohmann::json;

class ProblemList
{
public:
    // 局部静态特性的方式实现单实例
    static ProblemList *GetInstance();

    // 管理员查询题目详细信息
    Json SelectProblemInfoByAdmin(Json &queryjson);

    // 用户查询题目详细信息
    Json SelectProblem(Json &queryjson);

    // 插入题目
    Json InsertProblem(Json &insertjson);

    // 修改题目
    Json UpdateProblem(Json &updatejson);

    // 删除题目
    Json DeleteProblem(Json &deletejson);

    // 通过普通查询获取题库数据
    Json SelectProblemList(Json &queryjson);

    // 管理员查询列表
    Json SelectProblemListByAdmin(Json &queryjson);

    // 更新题目状态数量
    bool UpdateProblemStatusNum(Json &updatejson);

private:
    ProblemList();
    ~ProblemList();
};

#endif