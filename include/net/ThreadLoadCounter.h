#pragma once
/**
* cpu负载计算器
*/
#include <cstdint>
#include <mutex>
#include <list>
#include "Timestamp.h"

class ThreadLoadCounter {
public:
    /**
     * 构造函数
     * @param max_size 统计样本数量
     * @param max_usec 统计时间窗口,亦即最近{max_usec}的cpu负载率
     */
    ThreadLoadCounter(uint64_t max_size = 32, uint64_t max_usec = 2 * 1000 * 1000);
    ~ThreadLoadCounter() = default;

    /**
     * 线程进入休眠
     */
    void startSleep();

    /**
     * 休眠唤醒,结束休眠
     */
    void sleepWakeUp();

    /**
     * 返回当前线程cpu使用率，范围为 0 ~ 100
     * @return 当前线程cpu使用率
     */
    int load();

private:
    struct TimeRecord {
        TimeRecord(uint64_t tm, bool slp) {
            _time = tm;
            _sleep = slp;
        }

        bool _sleep;
        uint64_t _time;
    };

private:
    bool _sleeping = true;
    uint64_t _last_sleep_time;
    uint64_t _last_wake_time;
    uint64_t _max_size;
    uint64_t _max_usec;
    std::mutex _mtx;
    std::list<TimeRecord> _time_list;
};