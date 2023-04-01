#include "Logging.h"

namespace ThreadInfo {
__thread char t_errnobuf[512];
__thread char t_time[512];
__thread time_t t_lastSecond;
} // namespace ThreadInfo

const char *getErrnoMsg(int savedErrno) {
  return strerror_r(savedErrno, ThreadInfo::t_errnobuf,
                    sizeof(ThreadInfo::t_errnobuf));
}

const char *getLevelName[Logger::LogLevel::LEVEL_COUNT]{
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

Logger::LogLevel initLogLevel() { return Logger::LogLevel::INFO; }

Logger::LogLevel g_logLevel = initLogLevel();

static void defaultOutput(const char *data, int len) {
  fwrite(data, len, sizeof(data), stdout);
}

static void defaultFlush() {
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, int savedErrno, const char *file, int line)
  : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file)

{
  formatTime();
  stream_ << GeneralTemplate(getLevelName[level], 6);
  if(savedErrno != 0) {
    stream_ << getErrnoMsg(savedErrno) << "(errno=" << savedErrno << ")";
  }
}

void Logger::Impl::formatTime() {
  Timestamp now = Timestamp::now();
  auto seconds = static_cast<time_t> (now.microSecondsSinceEpoch() / Timestamp::kMicroSecondsPerSecond);
  auto microseconds = static_cast<int>(now.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond);
  struct tm* tm_time = localtime(&seconds);
  snprintf(ThreadInfo::t_time, sizeof(ThreadInfo::t_time), "%4d%02d%02d %02d:%02d:%02d",
           tm_time->tm_year+1900,
           tm_time->tm_mon + 1,
           tm_time->tm_mday,
           tm_time->tm_hour,
           tm_time->tm_min,
           tm_time->tm_sec);

  ThreadInfo::t_lastSecond = seconds;
  char buf[32] = {0};
  snprintf(buf, sizeof(buf), "%06d", microseconds);
  stream_ << GeneralTemplate(ThreadInfo::t_time, 17) << GeneralTemplate(buf, 7);
}

void Logger::Impl::finish() {
  stream_ << " - " << GeneralTemplate(basename_.data_, basename_.size_) << ':' << line_ << '\n';
}

Logger::Logger(const char *file, int line)
  : impl_(INFO, 0, file, line)
{
}

Logger::Logger(const char *file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{
}

Logger::Logger(const char *file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line)
{
  impl_.stream_ << func << ' ';
}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if(impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::setOutput(OutputFunc out){
  g_output = std::move(out);
}

void Logger::setFlush(FlushFunc flush) {
  g_flush = std::move(flush);
}

void Logger::setLogLevel(Logger::LogLevel level) {
  g_logLevel = level;
}
