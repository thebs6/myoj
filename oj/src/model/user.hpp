#pragma once
#include <iostream>
using string = std::string;
class User {
private:
    long id;
    string username;
    string nickname;
    string password;
    string school;
    string major;
    int acnum;
    int submitnum;
public:
    // 默认构造函数
    User() = default;

    // 带参数的构造函数
    User(long id, const string& username, const string& nickname, const string& password,
         const string& school, const string& major, int acnum, int submitnum)
        : id(id), username(username), nickname(nickname), password(password),
          school(school), major(major), acnum(acnum), submitnum(submitnum) {}


    long getId() const { return id; }
    void setId(const long& value) { id = value; }

    string getUsername() const { return username; }
    void setUsername(const string& value) { username = value; }

    string getNickname() const { return nickname; }
    void setNickname(const string& value) { nickname = value; }

    string getPassword() const { return password; }
    void setPassword(const string& value) { password = value; }

    string getSchool() const { return school; }
    void setSchool(const string& value) { school = value; }

    string getMajor() const { return major; }
    void setMajor(const string& value) { major = value; }

    int getAcnum() const { return acnum; }
    void setAcnum(const int& value) { acnum = value; }

    int getSubmitnum() const { return submitnum; }
    void setSubmitnum(const int& value) { submitnum = value; }

};
