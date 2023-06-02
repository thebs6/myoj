#include "ThreadLoadCounter.h"
#include <algorithm>

ThreadLoadCounter::ThreadLoadCounter(uint64_t max_size, uint64_t max_usec) {
    _last_sleep_time = _last_wake_time = Timestamp::now().microSecondsSinceEpoch();
    _max_size = max_size;
    _max_usec = max_usec;
}

void ThreadLoadCounter::startSleep() {
    std::lock_guard<std::mutex> lck(_mtx);
    _sleeping = true;
    auto current_time = Timestamp::now().microSecondsSinceEpoch();
    auto run_time = current_time - _last_wake_time;
    _last_sleep_time = current_time;
    _time_list.emplace_back(run_time, false);
    if (_time_list.size() > _max_size) {
        _time_list.pop_front();
    }
}

void ThreadLoadCounter::sleepWakeUp() {
    std::lock_guard<std::mutex> lck(_mtx);
    _sleeping = false;
    auto current_time = Timestamp::now().microSecondsSinceEpoch();
    auto sleep_time = current_time - _last_sleep_time;
    _last_wake_time = current_time;
    _time_list.emplace_back(sleep_time, true);
    if (_time_list.size() > _max_size) {
        _time_list.pop_front();
    }
}


int ThreadLoadCounter::load() {
    // std::lock_guard<std::mutex> lck(_mtx);
    uint64_t totalSleepTime = 0;
    uint64_t totalRunTime = 0;

    std::for_each(_time_list.begin(), _time_list.end(), [&](const TimeRecord &rcd) {
        if (rcd._sleep) {
            totalSleepTime += rcd._time;
        } else {
            totalRunTime += rcd._time;
        }
    });
    // _time_list.for_each();

    if (_sleeping) {
        totalSleepTime += (Timestamp::now().microSecondsSinceEpoch() - _last_sleep_time);
    } else {
        totalRunTime += (Timestamp::now().microSecondsSinceEpoch() - _last_wake_time);
    }

    uint64_t totalTime = totalRunTime + totalSleepTime;
    while ((_time_list.size() != 0) && (totalTime > _max_usec || _time_list.size() > _max_size)) {
        TimeRecord &rcd = _time_list.front();
        if (rcd._sleep) {
            totalSleepTime -= rcd._time;
        } else {
            totalRunTime -= rcd._time;
        }
        totalTime -= rcd._time;
        _time_list.pop_front();
    }
    if (totalTime == 0) {
        return 0;
    }
    return (int) (totalRunTime * 100 / totalTime);
}