#ifndef GLFUN_MATH_UTIL_H
#define GLFUN_MATH_UTIL_H

#include <type_traits>

constexpr double Pi_d = 3.1415926535897932384626433832795028841972;
constexpr float Pi_f = Pi_d;

constexpr double Tau_d = 2 * Pi_d;
constexpr float Tau_f = Tau_d;

constexpr double Root2_d = 1.4142135623730950488016887242096980785697;
constexpr float Root2_f = Root2_d;

// Linearly map x from the range [x1,x2] to the range [y1,y2]
template<typename T>
T LinearMap(T x, T x1, T x2, T y1, T y2) {
  T slope = (y2 - y1) / (x2 - x1);
  T intercept = y1 - slope * x1;
  return slope * x + intercept;
}

inline float LinearMap_f(float x, float x1, float x2, float y1, float y2) {
  return LinearMap(x, x1, x2, y1, y2);
}

inline double LinearMap_d(
  double x, double x1, double x2, double y1, double y2
) {
  return LinearMap(x, x1, x2, y1, y2);
}

template<typename T>
constexpr bool IsPowerOf2(T x) {
  if(x <= 0) return false;
  return (x & (x - 1)) == 0; // check that exactly 1 bit is set
}

// e.g. Log<2>(64) == 6. rounds down.
template<int Base, typename T>
constexpr T Log(T x) {
  static_assert(std::is_integral<T>::value);

  if(x < 1)
    return -1;

  T log = 0;
  while(x > 1) {
    x /= Base;
    log++;
  }
  return log;
}

#endif
