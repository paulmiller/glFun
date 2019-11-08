#ifndef UTIL_H
#define UTIL_H

#include <ostream>
#include <string>
#include <vector>

bool hasPrefix(const char *prefix, const char *str);

std::string readWholeFileOrThrow(const char *file_name);

// e.g. PrettyPrintNumBytes(4096) -> "4 KiB" (rounds to whole numbers)
std::string PrettyPrintNumBytes(unsigned long long n);

// TODO a generic join function
template<typename T>
std::ostream& operator<<(std::ostream &os, const std::vector<T> &v) {
  os << '[';
  bool first = true;
  for(const T &e: v) {
    if(first)
      first = false;
    else
      os << ' ';
    os << e;
  }
  os << ']';
  return os;
}

#endif
