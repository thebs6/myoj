#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <iostream>

// 封装sockaddr_in
class InetAddress
{
public:
    // explicit 防止构造函数的隐式转换
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    std::string toIp() const;
    uint16_t toPort() const;
    std::string toIpPort() const;

    inline const sockaddr_in* getSockAddr() const { return &addr_; };
    inline void setSockAddr(const sockaddr_in& addr) { addr_ = addr; };
private:
    sockaddr_in addr_;
};