#ifndef SOLUTIONLIST_H
#define SOLUTIONLIST_H
#include "json.hpp"
using Json = nlohmann::json;
#include <string>
// 讨论类
class SolutionList
{
public:
    // 局部静态特性的方式实现单实例
    static SolutionList *GetInstance();

    // 添加题解
    Json InsertSolution(Json &insertjson);

    // 分页查询题解
    Json SelectSolutionList(Json &queryjson);

    // 管理员查询
    Json SelectSolutionListByAdmin(Json &queryjson);

    // 查询
    Json SelectSolutionByEdit(Json &queryjson);

    // 查询题解的内容
    Json SelectSolution(Json &queryjson);

    // 修改评论数的数量
    bool UpdateSolutionComments(Json &updatejson);

    // 更新题解
    Json UpdateSolution(Json &updatejson);

    // 删除题解
    Json DeleteSolution(Json &deletejson);

private:
    SolutionList();
    ~SolutionList();
};
#endif