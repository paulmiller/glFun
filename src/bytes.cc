#include "bytes.h"

#include "ohno.h"

#include <cassert>
#include <cstring> // for memset
#include <fstream>
#include <iostream>

Bytes::Bytes(size_t size) : mSize(size) {
  mBytes = new char[size];
  memset(mBytes, 0, size);
}

// TODO not called?
Bytes::Bytes(Bytes&& src) : mBytes(src.mBytes), mSize(src.mSize) {
  src.mBytes = nullptr;
}

Bytes::~Bytes() {
  delete[] mBytes;
}

char *Bytes::get() {
  return mBytes;
}

size_t Bytes::size() const {
  return mSize;
}

char Bytes::operator[](size_t i) const {
  assert(i < mSize);
  return mBytes[i];
}

char& Bytes::operator[](size_t i) {
  assert(i < mSize);
  return mBytes[i];
}

Bytes Bytes::loadFile(const char *fileName) {
  std::ifstream input(fileName, std::ifstream::binary);
  if(input.bad())
    throw OHNO("couldn't open file");

  input.seekg(0, input.end);
  size_t size = input.tellg();
  input.seekg(0, input.beg);

  Bytes bytes(size + 1); // +1 for null-terminator
  input.read(bytes.get(), size);
  if(input.bad())
    throw OHNO("couldn't read file");
  return bytes;
}
