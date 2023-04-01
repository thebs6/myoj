#pragma once

#include "FixedBuffer.h"
#include "noncopyable.h"
#include <iostream>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

class GeneralTemplate : noncopyable {
public:
  GeneralTemplate() = default;

  explicit GeneralTemplate(const char *data, int len)
      : data_(data), len_(len) {}
  const char *data_;
  int len_;

private:
};

class LogStream : noncopyable {
public:
  using Buffer = FixedBuffer<kSmallBuffer>;

  void append(const char *data, int len) { buffer_.append(data, len); }
  const Buffer &buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

  LogStream &operator<<(short);
  LogStream &operator<<(unsigned short);
  LogStream &operator<<(int);
  LogStream &operator<<(unsigned int);
  LogStream &operator<<(long);
  LogStream &operator<<(unsigned long);
  LogStream &operator<<(long long);
  LogStream &operator<<(unsigned long long);

  LogStream &operator<<(float v);
  LogStream &operator<<(double v);

  LogStream &operator<<(char);
  LogStream &operator<<(const char *str);
  LogStream &operator<<(const void *data);
  LogStream &operator<<(const unsigned char *str);
  LogStream &operator<<(const std::string &str);
  LogStream &operator<<(const Buffer &buf);

  LogStream &operator<<(const GeneralTemplate &g);

private:
  static const int kMaxNumericSize = 48;
  template <typename T> void formatInteger(T);

  Buffer buffer_;
};

class Fmt {
public:
  template <typename T> Fmt(const char *fmt, T val);
  const char *data() const { return buf_; }
  int length() const { return length_; }
private:
  char buf_[32];
  int length_;
};