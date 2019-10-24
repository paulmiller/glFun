#ifndef UTIL_H
#define UTIL_H

#include <string>

bool hasPrefix(const char *prefix, const char *str);

std::string readWholeFileOrThrow(const char *file_name);

// e.g. PrettyPrintNumBytes(4096) -> "4 KiB" (rounds to whole numbers)
std::string PrettyPrintNumBytes(unsigned long long n);

#endif
