#pragma once

class Logger {
public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    LEVEL_COUNT,
  };

  Logger(const char *file, int line);
  Logger(const char *file, int line, LogLevel level);
  Logger(const char *file, int line, LogLevel level, const char *func);
  ~Logger();

  LogStream &stream() {}

private:
  class Impl {
  public:
    using
  };
};

#define LOG_DEBUG if (logLevel() <=)