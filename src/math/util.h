#ifndef GLFUN_MATH_UTIL_H
#define GLFUN_MATH_UTIL_H

constexpr double Pi_d = 3.1415926535897932384626433832795028841972;
constexpr float Pi_f = Pi_d;

constexpr double Tau_d = 2 * Pi_d;
constexpr float Tau_f = Tau_f;

constexpr double Root2_d = 1.4142135623730950488016887242096980785697;
constexpr float Root2_f = Root2_d;

template<typename T>
T invSqrt(T x) {
  return 1 / sqrt(x);
}

// Linearly map x from the range [x1,x2] to the range [y1,y2]
template<typename T>
T linearMap(T x, T x1, T x2, T y1, T y2) {
  T slope = (y2 - y1) / (x2 - x1);
  T intercept = y1 - slope * x1;
  return slope * x + intercept;
}

template<typename T>
T radToDeg(T radians) {
  return radians * (180 / Pi_f);
}

template<typename T>
T degToRad(T degrees) {
  return degrees * (Pi_f / 180);
}

#endif
