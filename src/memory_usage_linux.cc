#include "memory_usage.h"

#include "util.h"

#include <iostream>
#include <unistd.h>

void PrintMemoryUsage() {
  unsigned long long size, resident, shared, text, lib, data, dt;
  std::string statm = readWholeFileOrThrow("/proc/self/statm");
  sscanf(statm.c_str(), "%llu %llu %llu %llu %llu %llu %llu",
    &size, &resident, &shared, &text, &lib, &data, &dt);
  int page_size = getpagesize();
  std::cout << "memory usage: size=" << PrettyPrintNumBytes(size * page_size)
    << " resident=" << PrettyPrintNumBytes(resident * page_size)
    << " text=" << PrettyPrintNumBytes(text * page_size)
    << " data=" << PrettyPrintNumBytes(data * page_size) << '\n';
}
