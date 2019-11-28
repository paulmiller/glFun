#ifndef COLOR_H
#define COLOR_H

#include <algorithm>
#include <cstdint>

class Color {
public:
  static const Color Black;
  static const Color White;
  
  // r,g,b ∈ [0,1]
  float r, g, b;

  uint8_t RByte() const { return uint8_t(std::clamp(r * 255.f, 0.f, 255.f)); }
  uint8_t GByte() const { return uint8_t(std::clamp(g * 255.f, 0.f, 255.f)); }
  uint8_t BByte() const { return uint8_t(std::clamp(b * 255.f, 0.f, 255.f)); }
};

// h,s,v ∈ [0,1]; (0,1,1) and (1,1,1) are both bright red
Color ColorFromHsv(float h, float s, float v);

#endif
