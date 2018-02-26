#ifndef UTIL_H
#define UTIL_H

// Based on BUILD_BUG_ON from kernel.h
#define COMPILE_ASSERT(condition) ((void) sizeof(char[1 - 2*!(condition)]))

constexpr double PI_d = 3.1415926535897932384626433832795028841971;
constexpr float  PI_f = PI_d;

float invSqrt(float x);

// Linearly map x from the range [x1,x2] to the range [y1,y2]
float linearMap(float x, float x1, float x2, float y1, float y2);

float radToDeg(float radians);

float degToRad(float degrees);

bool hasPrefix(const char *prefix, const char *str);

#endif
