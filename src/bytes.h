#ifndef BYTES_H
#define BYTES_H

#include <cstddef>

class Bytes {
private:
  char *mBytes;
  size_t mSize;
public:
  Bytes(size_t size);
  Bytes(Bytes&& src);
  ~Bytes();
  char *get();
  size_t size() const;
  char operator[](size_t i) const;
  char& operator[](size_t i);
  static Bytes loadFile(const char *fileName); // with extra null-terminator
};

#endif
