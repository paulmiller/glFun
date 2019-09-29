#ifndef UTIL_H
#define UTIL_H

#include <string>

// Based on BUILD_BUG_ON from kernel.h
#define COMPILE_ASSERT(condition) ((void) sizeof(char[1 - 2*!(condition)]))

bool hasPrefix(const char *prefix, const char *str);

std::string readWholeFileOrThrow(const char *file_name);

#endif
