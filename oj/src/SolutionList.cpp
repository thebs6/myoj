#include "SolutionList.h"
#include "MongoDataBase.h"
// 局部静态特性的方式实现单实例
SolutionList *SolutionList::GetInstance()
{
    static SolutionList solutionlist;
    return &solutionlist;
}

Json SolutionList::InsertSolution(Json &insertjson)
{
    return MoDB::GetInstance()->InsertSolution(insertjson);
}

Json SolutionList::SelectSolutionList(Json &queryjson)
{
    return MoDB::GetInstance()->SelectSolutionList(queryjson);
}

Json SolutionList::SelectSolutionListByAdmin(Json &queryjson)
{
    return MoDB::GetInstance()->SelectSolutionListByAdmin(queryjson);
}

Json SolutionList::SelectSolutionByEdit(Json &queryjson)
{
    return MoDB::GetInstance()->SelectSolutionByEdit(queryjson);
}

Json SolutionList::SelectSolution(Json &queryjson)
{
    return MoDB::GetInstance()->SelectSolution(queryjson);
}

bool SolutionList::UpdateSolutionComments(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateSolutionComments(updatejson);
}

Json SolutionList::UpdateSolution(Json &updatejson)
{
    return MoDB::GetInstance()->UpdateSolution(updatejson);
}

Json SolutionList::DeleteSolution(Json &deletejson)
{
    return MoDB::GetInstance()->DeleteSolution(deletejson);
}

SolutionList::SolutionList()
{
}
SolutionList::~SolutionList()
{
}