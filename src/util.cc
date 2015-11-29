#include "util.h"

#include <cassert>
#include <cmath>

float invSqrt(float x) {
  return 1 / sqrt(x);
}

float linearMap(float x, float x1, float x2, float y1, float y2) {
  float slope = (y2 - y1) / (x2 - x1);
  float intercept = y1 - slope * x1;
  return slope * x + intercept;
}

float radToDeg(float radians) {
  return radians * (180 / PI);
}

float degToRad(float degrees) {
  return degrees * (PI / 180);
}

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

