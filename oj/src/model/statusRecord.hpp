#pragma once
#include <iostream>
using string = std::string;
class StatusRecord {
private:
    long id;
    long problem;
    long username;
    string nickname;
    string problem_title;
    int status;
    string run_time;
    string run_memory;
    string length;
    string language;
    string submit_time;
    string codecompiler_info;
    string code;
public:
    StatusRecord() = default;
    StatusRecord(long id, long problem, long username, const string& nickname, const string& problem_title, int status, const string& run_time, const string& run_memory, const string& length, const string& language, const string& submit_time, const string& codecompiler_info, const string& code)
        : id(id), problem(problem), username(username), nickname(nickname), problem_title(problem_title), status(status), run_time(run_time), run_memory(run_memory), length(length), language(language), submit_time(submit_time), codecompiler_info(codecompiler_info), code(code) {
        // 构造函数的实现代码
    }

    long getId() const { return id; }
    void setId(const long& value) { id = value; }

    long getProblem() const { return problem; }
    void setProblem(const long& value) { problem = value; }

    long getUsername() const { return username; }
    void setUsername(const long& value) { username = value; }

    string getNickname() const { return nickname; }
    void setNickname(const string& value) { nickname = value; }

    string getProblem_title() const { return problem_title; }
    void setProblem_title(const string& value) { problem_title = value; }

    int getStatus() const { return status; }
    void setStatus(const int& value) { status = value; }

    string getRun_time() const { return run_time; }
    void setRun_time(const string& value) { run_time = value; }

    string getRun_memory() const { return run_memory; }
    void setRun_memory(const string& value) { run_memory = value; }

    string getLength() const { return length; }
    void setLength(const string& value) { length = value; }

    string getLanguage() const { return language; }
    void setLanguage(const string& value) { language = value; }

    string getSubmit_time() const { return submit_time; }
    void setSubmit_time(const string& value) { submit_time = value; }

    string getCodecompiler_info() const { return codecompiler_info; }
    void setCodecompiler_info(const string& value) { codecompiler_info = value; }

    string getCode() const { return code; }
    void setCode(const string& value) { code = value; }
};