#include "ProblemList.h"
#include "MongoDataBase.h"
#include "MysqlDataBase.h"
#include "RedisDataBase.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

const string PROBLEMDATAPATH = "../../problemdata/";

ProblemList::ProblemList()
{
}

Json ProblemList::SelectProblemInfoByAdmin(Json &queryjson)
{
    // 获取基本信息
    Json resjson = MoDB::GetInstance()->SelectProblemInfoByAdmin(queryjson);

    if (resjson["Result"].get<std::string>() == "Fail")
    {
        return resjson;
    }
    // 获取测试点信息
    string problemid = resjson["_id"].get<std::string>();
    int judgenum = stoi(resjson["JudgeNum"].get<std::string>());
    string DATA_PATH = PROBLEMDATAPATH + problemid + "/";
    ifstream infilein, infileout;
    for (int i = 1; i <= judgenum; i++)
    {
        Json jsoninfo;
        string infilepath = DATA_PATH + to_string(i) + ".in";
        string outfilepath = DATA_PATH + to_string(i) + ".out";

        infilein.open(infilepath.data());
        string infile((istreambuf_iterator<char>(infilein)),
                      (istreambuf_iterator<char>()));

        infileout.open(outfilepath.data());
        string outfile((istreambuf_iterator<char>(infileout)),
                       (istreambuf_iterator<char>()));
        jsoninfo["in"] = infile;
        jsoninfo["out"] = outfile;

        infilein.close();
        infileout.close();

        resjson["TestInfo"].push_back(jsoninfo);
    }
    // 获取SPJ文件
    resjson["IsSPJ"] = false;
    string spjpath = DATA_PATH + "spj.cpp";
    if (access(spjpath.data(), F_OK) == 0)
    {
        ifstream infilespj;
        infilespj.open(spjpath.data());
        string spjfile((istreambuf_iterator<char>(infilespj)),
                       (istreambuf_iterator<char>()));
        resjson["SPJ"] = spjfile;
        resjson["IsSPJ"] = true;
        infilespj.close();
    }
    return resjson;
}

Json ProblemList::SelectProblem(Json &queryjson)
{
    string problemid = queryjson["ProblemId"].get<std::string>();

    // 获取缓存
    string resstr = ReDB::GetInstance()->GetProblemCache(problemid);

    Json resjson;
    
    // 如果有缓存
    if (resstr != "")
    {
        // 解析缓存json
        resjson = Json::parse(resstr);

        return resjson;
    }
    // 如果没有缓存
    resjson = MoDB::GetInstance()->SelectProblem(queryjson);

    // 添加缓存
    if (resjson["Result"].get<std::string>() == "Success")
    {
        ReDB::GetInstance()->AddProblemCache(problemid, resjson.dump());
    }

    return resjson;
}

bool InsertProblemDataInfo(Json &insertjson)
{
    // 添加测试用例
    string DATA_PATH = PROBLEMDATAPATH + insertjson["ProblemId"].get<std::string>();
    string command = "mkdir " + DATA_PATH;
    // 创建文件夹
    system(command.data());
    // 添加测试文件
    for (int i = 1; i <= insertjson["TestInfo"].size(); i++)
    {
        string index = to_string(i);
        ofstream outfilein, outfileout;
        string inpath = DATA_PATH + "/" + index + ".in";
        string outpath = DATA_PATH + "/" + index + ".out";

        outfilein.open(inpath.data());
        outfilein << insertjson["TestInfo"][i - 1]["in"].get<std::string>();
        outfilein.close();

        outfileout.open(outpath.data());
        outfileout << insertjson["TestInfo"][i - 1]["out"].get<std::string>();
        outfileout.close();
    }
    // 添加SPJ文件
    if (insertjson["IsSPJ"].get<bool>())
    {
        ofstream outfilespj;
        string spjpath = DATA_PATH + "/spj.cpp";
        outfilespj.open(spjpath.data());
        outfilespj << insertjson["SPJ"].get<std::string>();
        outfilespj.close();
    }
    return true;
}
Json ProblemList::InsertProblem(Json &insertjson)
{
    Json tmpjson = MoDB::GetInstance()->InsertProblem(insertjson);

    if (tmpjson["Result"] == "Fail") // 插入失败
        return tmpjson;

    // 插入信息
    Json problemjson;
    problemjson["_id"] = tmpjson["ProblemId"];
    problemjson["Title"] = insertjson["Title"];
    problemjson["Description"] = insertjson["Description"];
    problemjson["JudgeNum"] = insertjson["JudgeNum"];
    problemjson["TimeLimit"] = insertjson["TimeLimit"];
    problemjson["MemoryLimit"] = insertjson["MemoryLimit"];

    insertjson["ProblemId"] = tmpjson["ProblemId"];

    InsertProblemDataInfo(insertjson);
    return tmpjson;
}

Json ProblemList::UpdateProblem(Json &updatejson)
{
    Json tmpjson = MoDB::GetInstance()->UpdateProblem(updatejson);
    if (tmpjson["Result"].get<std::string>() == "Fail")
        return tmpjson;

    string problemid = updatejson["ProblemId"].get<std::string>();
    string DATA_PATH = PROBLEMDATAPATH + problemid;
    // 删除文件夹
    string command = "rm -rf " + DATA_PATH;
    system(command.data());
    // 创建文件夹
    InsertProblemDataInfo(updatejson);

    // 删除缓存
    ReDB::GetInstance()->DeleteProblemCache(problemid);
    return tmpjson;
}

Json ProblemList::DeleteProblem(Json &deletejson)
{
    Json tmpjson = MoDB::GetInstance()->DeleteProblem(deletejson);

    if (tmpjson["Result"] == "Fail")
        return tmpjson;

    // 删除数据
    string DATA_PATH = PROBLEMDATAPATH + deletejson["ProblemId"].get<std::string>();
    string command = "rm -rf " + DATA_PATH;

    system(command.data());

    // 删除缓存
    ReDB::GetInstance()->DeleteProblemCache(deletejson["ProblemId"].get<std::string>());
    return tmpjson;
}

Json ProblemList::SelectProblemList(Json &queryjson)
{
    return MoDB::GetInstance()->SelectProblemList(queryjson);
}

Json ProblemList::SelectProblemListByAdmin(Json &queryjson)
{
    return MoDB::GetInstance()->SelectProblemListByAdmin(queryjson);
}

bool ProblemList::UpdateProblemStatusNum(Json &updatejson)
{
    return MysqlDataBase::GetInstance()->UpdateProblemStatusNum(updatejson);
}

ProblemList *ProblemList::GetInstance()
{
    static ProblemList problemlist;
    return &problemlist;
}

ProblemList::~ProblemList()
{
}