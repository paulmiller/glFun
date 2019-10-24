#include "util.h"

#include "ohno.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

bool hasPrefix(const char *prefix, const char *str) {
  assert(prefix);
  assert(str);
  for(std::size_t i = 0; prefix[i]; i++) {
    if(prefix[i] != str[i]) {
      return false;
    }
  }
  return true;
}

std::string readWholeFileOrThrow(const char *file_name) {
  std::ifstream file_stream(file_name, std::ifstream::in);
  std::stringstream str_stream;
  str_stream << file_stream.rdbuf();
  if(file_stream.fail() || str_stream.fail()) {
    std::cout << "failed reading file \"" << file_name << "\"\n";
    throw OHNO("");
  }
  return str_stream.str();
}

std::string PrettyPrintNumBytes(unsigned long long num) {
  static const char * const units[] =
    {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

  // must add bigger units if ullong can represent values >= 1024 EiB
  static_assert(std::numeric_limits<decltype(num)>::digits <= 70);

  int unit = 0;
  bool round_up = false;
  while(num + round_up >= 1024) {
    round_up = ((num & 1023) >= 512);
    num >>= 10;
    unit++;
  }
  assert(unit < std::size(units));

  std::stringstream out;
  out << (num + round_up) << ' ' << units[unit];
  return out.str();
}
