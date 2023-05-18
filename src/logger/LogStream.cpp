#include "LogStream.h"
#include <iostream>
#include <algorithm>
static const char digits[] = "9876543210123456789";

template <typename T>
void LogStream::formatInteger(T num) {
  if(buffer_.avail() >= kMaxNumericSize) {
    char* start = buffer_.current();
    char* cur = start;
    const char* zero = digits + 9;
    bool negative = (num < 0);
    do {
      int remainder = static_cast<int>(num % 10);
      *(cur++) = zero[remainder];
      num = num / 10;
    } while(num != 0);

    if(negative) {
      *(cur++) = '-';
    }
    *cur = '\0';
    std::reverse(start, cur);
    buffer_.add(static_cast<int> (cur - start));
  }
}


LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int> (v);
  return *this;
}
LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int> (v);
  return *this;
}
LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    *this << static_cast<double> (v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
  if(buffer_.avail() >= kMaxNumericSize) {
    char buf[32];
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
    buffer_.add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(char c) {
  buffer_.append(&c, 1);
  return *this;
}
LogStream& LogStream::operator<<(const char *str) {
  if(str != nullptr) {
    buffer_.append(str, strlen(str));
  } else {
    buffer_.append("(null)", 6);
  }
  return *this;
}

LogStream& LogStream::operator<<(const void *data) {
  *this << static_cast<const char*> (data);
  return *this;
}
LogStream& LogStream::operator<<(const unsigned char *str) {
  *this << reinterpret_cast<const char*>(str);
  return *this;
}
LogStream& LogStream::operator<<(const std::string &str) {
  buffer_.append(str.c_str(), str.length());
  return *this;
}
LogStream& LogStream::operator<<(const Buffer &buf) {
  *this << buf.toString();
  return *this;
}

LogStream& LogStream::operator<<(const GeneralTemplate &g) {
  buffer_.append(g.data_, g.len_);
  return *this;
}


template <typename T>
Fmt::Fmt(const char* fmt, T val) {
  static_assert(std::is_arithmetic<T>::value, "Must be arithmetic type");
  length_ = snprintf(buf_, sizeof(buf_), fmt, val);
  // assert!
  //  (static_cast<size_t> (length_) < sizeof(buf_));
}


template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);

template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
