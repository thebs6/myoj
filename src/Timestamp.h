#pragma once

#include <iostream>

class Timestamp
{
public:
    explicit Timestamp();
    Timestamp(int64_t microSecondSinceEpoch);
    inline static Timestamp now() {return Timestamp(time(NULL));}
    std::string toString() const;
    void swap(Timestamp& that);
private:
    int64_t microSecondSinceEpoch_;
};