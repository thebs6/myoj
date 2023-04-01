#pragma once
#include "FixedBuffer.h"
#include "Thread.h"

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "LogStream.h"

class AsyncLogging {
public:
  AsyncLogging(const std::string& basename,
               off_t roolSize,
               int flushInterval = 3
               );
  ~AsyncLogging()
  {
    if(running_)
    {
      stop();
    }
  }

  void append(const char* logLine, int len);

  void start()
  {
    running_ = true;
    thread_.start();
  }

  void stop() {
   running_ = false;
   cond_.notify_one();
   thread_.join();
  }

private:
  using Buffer = FixedBuffer<kLargeBuffer>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = BufferVector::value_type;

  void threadFunc();

  const int flushInterval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t rollSize_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;

  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};
