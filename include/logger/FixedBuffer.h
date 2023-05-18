#pragma once
#include <string.h>
#include <iostream>
template <int SIZE>
class FixedBuffer {
public:
  FixedBuffer() : cur_(data_)
  {
  }
  ~FixedBuffer() = default;

  void append(const char* buf, size_t len) {
    if(static_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return static_cast<int> (cur_ - data_); }

  char* current() const { return cur_; }
  int avail() const { return static_cast<int> (end() - cur_); }
  void add(size_t len) { cur_ += len;}

  void reset() { cur_ = data_; }
  void bzero() { memset(data_, 0, sizeof(data_)); }

  std::string toString() const { return std::string(data_, length()); }
private:
  char data_[SIZE];
  char* cur_;
  const char* end() const { return data_ + sizeof(data_); }
};