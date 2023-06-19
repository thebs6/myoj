#pragma once
#include "json.hpp"
#include <string>
#include <vector>

using Json = nlohmann::json;

class User {
public:
    long long _id;
    std::string avatar;
    std::string nick_name;
    std::string account;
    std::string password;
    std::string personal_profile;
    std::string school;
    std::string major;
    std::string join_time;
    std::vector<std::string> comment_likes;
    std::vector<int> solves;
    int ac_num;
    int submit_num;
    int authority;

    // Constructor
    User(long long id, const std::string& avatar, const std::string& nickName, const std::string& acc,
         const std::string& pass, const std::string& profile, const std::string& sch, const std::string& maj,
         const std::string& join, const std::vector<std::string>& likes, const std::vector<int>& sol,
         int acNum, int subNum, int auth)
        : _id(id), avatar(avatar), nick_name(nickName), account(acc), password(pass), personal_profile(profile),
          school(sch), major(maj), join_time(join), comment_likes(likes), solves(sol),
          ac_num(acNum), submit_num(subNum), authority(auth) {}

public:
};
