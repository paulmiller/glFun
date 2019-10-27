#ifndef COLOR_H
#define COLOR_H

#include <algorithm>

class Color {
public:
  static const Color Black;
  static const Color White;
  
  float r, g, b;

  uint8_t RByte() const { return uint8_t(std::clamp(r * 256.f, 0.f, 255.f)); }
  uint8_t GByte() const { return uint8_t(std::clamp(g * 256.f, 0.f, 255.f)); }
  uint8_t BByte() const { return uint8_t(std::clamp(b * 256.f, 0.f, 255.f)); }
};

#endif
