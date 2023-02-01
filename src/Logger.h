#pragma once

#include <cstdio>
#include <iostream>

#include "noncopyable.h"

#define MUTEDEBUG

#define LOG_INFO(logmsgFormat, ...)                         \
    do                                                      \
    {                                                       \
        Logger& logger = Logger::instance();                \
        logger.setLogLevel(INFO);                           \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    } while(0);  

#define ERROR_INFO(logmsgFormat, ...)                         \
    do                                                      \
    {                                                       \
        Logger& logger = Logger::instance();                \
        logger.setLogLevel(ERROR);                           \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    } while(0);    

#define FATAL_INFO(logmsgFormat, ...)                         \
    do                                                      \
    {                                                       \
        Logger& logger = Logger::instance();                \
        logger.setLogLevel(FATAL);                           \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    } while(0);  
  
#ifdef MUTEDEBUG
#define DEBUG_INFO(logmsgFormat, ...)                         \
    do                                                      \
    {                                                       \
        Logger& logger = Logger::instance();                \
        logger.setLogLevel(DEBUG);                           \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    } while(0);                       
#else
#define DEBUG_INFO(logmsgFormat, ...)  
#endif

enum LogLevel
{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};

class Logger : noncopyable
{
public:
    static Logger& instance();
    inline void setLogLevel(int level) {level = LogLevel_;}
    void log(std::string);
private:
    int LogLevel_;
};