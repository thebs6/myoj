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
    int64_t microSecondsSinceEpoch() const { return microSecondSinceEpoch_; }
    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t microSecondSinceEpoch_;
};