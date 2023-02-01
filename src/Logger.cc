#include "Logger.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

// message : [LogLevel] {timestamp} : msg 
void Logger::log(std::string msg) 
{
    std::string pre = "";
    switch (LogLevel_) {
        case INFO : 
            pre = "[INFO]";
            break;
        case ERROR : 
            pre = "[ERROR]";
            break;
        case FATAL : 
            pre = "[FATAL]";
            break;
        case DEBUG : 
            pre = "[DEBUG]";
            break;
    }
    
    // TODO
    std::cout << pre <<"timestamp" << msg << std::endl;
}