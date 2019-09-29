#include "util.h"

#include "ohno.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
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
