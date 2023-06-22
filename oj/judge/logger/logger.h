#ifndef Y_JUDGER_LOGGER_H
#define Y_JUDGER_LOGGER_H
typedef int logType;

#define JFATAL       5
#define JERROR       4
#define JWARNING     3
#define JINFO        2
#define JDEBUG       1

void makeLog(logType type, char *content, FILE *loggerFile);

#endif //Y_JUDGER_LOGGER_H
