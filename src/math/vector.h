#ifndef GLFUN_MATH_VECTOR_H
#define GLFUN_MATH_VECTOR_H

#include <algorithm>
#include <cmath>
#include <ostream>

template<typename T> class Vector3;

typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;

extern const Vector3f Zero_Vector3f;
extern const Vector3d Zero_Vector3d;
extern const Vector3f UnitX_Vector3f;
extern const Vector3d UnitX_Vector3d;
extern const Vector3f UnitY_Vector3f;
extern const Vector3d UnitY_Vector3d;
extern const Vector3f UnitZ_Vector3f;
extern const Vector3d UnitZ_Vector3d;

template<typename T> class Vector4;

typedef Vector4<float> Vector4f;
typedef Vector4<double> Vector4d;

extern const Vector4f Zero_Vector4f;
extern const Vector4d Zero_Vector4d;
extern const Vector4f UnitX_Vector4f;
extern const Vector4d UnitX_Vector4d;
extern const Vector4f UnitY_Vector4f;
extern const Vector4d UnitY_Vector4d;
extern const Vector4f UnitZ_Vector4f;
extern const Vector4d UnitZ_Vector4d;
extern const Vector4f UnitW_Vector4f;
extern const Vector4d UnitW_Vector4d;

// --- Vector3 -------------------------------------------------------------- //

template<typename T>
class Vector3 {
public:
  T x, y, z;

  T len() const {
    return sqrt(len2());
  }

  T len2() const {
    return x*x + y*y + z*z;
  }

  Vector3 unit() const {
    T scale = 1 / len();
    return scale * *this;
  }
};

// --- Vector3 equality ----------------------------------------------------- //

// Invoking "T == T" may or may not be desired if T is floating-point. Use
// ApproxVector3 for approximate comparisons in Catch tests.

template<typename T>
inline bool operator==(const Vector3<T> &a, const Vector3<T> &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<typename T>
inline bool operator!=(const Vector3<T> &a, const Vector3<T> &b) {
  return !(a == b);
}

// --- Vector3 negation ----------------------------------------------------- //

template<typename T>
inline Vector3<T> operator-(const Vector3<T> &v) {
  return Vector3<T>{-v.x, -v.y, -v.z};
}

// --- Vector3 + Vector3 ---------------------------------------------------- //

template<typename T>
inline Vector3<T> operator+(const Vector3<T> &a, const Vector3<T> &b) {
  return Vector3<T> {a.x + b.x, a.y + b.y, a.z + b.z};
}

template<typename T>
inline Vector3<T>& operator+=(Vector3<T> &a, const Vector3<T> &b) {
  return a = a + b;
}

// --- Vector3 - Vector3 ---------------------------------------------------- //

template<typename T>
inline Vector3<T> operator-(const Vector3<T> &a, const Vector3<T> &b) {
  return Vector3<T> {a.x - b.x, a.y - b.y, a.z - b.z};
}

template<typename T>
inline Vector3<T>& operator-=(Vector3<T> &a, const Vector3<T> &b) {
  return a = a - b;
}

// --- Vector3 * scalar ----------------------------------------------------- //

template<typename T>
inline Vector3<T> operator*(const Vector3<T> &v, T scalar) {
  return Vector3<T>{v.x * scalar, v.y * scalar, v.z * scalar};
}

template<typename Vector_T, typename Scalar_T,
  std::enable_if_t<std::is_convertible<Scalar_T,Vector_T>::value, int> = 0>
inline Vector3<Vector_T> operator*(
  const Vector3<Vector_T> &v, Scalar_T scalar
) {
  return v * static_cast<Vector_T>(scalar);
}

template<typename T>
inline Vector3<T> operator*(T scalar, const Vector3<T> &v) {
  return Vector3<T>{scalar * v.x, scalar * v.y, scalar * v.z};
}

template<typename Vector_T, typename Scalar_T,
  std::enable_if_t<std::is_convertible<Scalar_T,Vector_T>::value, int> = 0>
inline Vector3<Vector_T> operator*(
  Scalar_T scalar, const Vector3<Vector_T> &v
) {
  return static_cast<Vector_T>(scalar) * v;
}

template<typename T>
inline Vector3<T>& operator*=(Vector3<T> &v, T scalar) {
  return v = v * scalar;
}

template<typename Vector_T, typename Scalar_T,
  std::enable_if_t<std::is_convertible<Scalar_T,Vector_T>::value, int> = 0>
inline Vector3<Vector_T> operator*=(
  const Vector3<Vector_T> &v, Scalar_T scalar
) {
  return v = v * static_cast<Vector_T>(scalar);
}

// --- Vector3 / scalar ----------------------------------------------------- //

template<typename T>
inline Vector3<T> operator/(const Vector3<T> &v, T scalar) {
  return Vector3<T>{v.x / scalar, v.y / scalar, v.z / scalar};
}

template<typename Vector_T, typename Scalar_T,
  std::enable_if_t<std::is_convertible<Scalar_T,Vector_T>::value, int> = 0>
inline Vector3<Vector_T> operator/(
  const Vector3<Vector_T> &v, Scalar_T scalar
) {
  return v / static_cast<Vector_T>(scalar);
}

template<typename T>
inline Vector3<T>& operator/=(Vector3<T> &v, T scalar) {
  return v = v / scalar;
}

template<typename Vector_T, typename Scalar_T,
  std::enable_if_t<std::is_convertible<Scalar_T,Vector_T>::value, int> = 0>
inline Vector3<Vector_T> operator/=(
  const Vector3<Vector_T> &v, Scalar_T scalar
) {
  return v = v / static_cast<Vector_T>(scalar);
}

// --- Vector3 special functions ------------------------------------------- //

template<typename T>
inline T dot(const Vector3<T> &a, const Vector3<T> &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
inline Vector3<T> cross(const Vector3<T> &a, const Vector3<T> &b) {
  return Vector3<T> {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

// projection of a onto b
template<typename T>
inline Vector3<T> proj(const Vector3<T> &a, const Vector3<T> &b) {
  Vector3<T> b1 = b.unit();
  return dot(a, b1) * b1;
}

template<typename T>
inline T angleBetween(const Vector3<T> &a, const Vector3<T> &b) {
  T x = dot(a, b) / (a.len() * b.len());

  // x should already be on [-1,1] (which is the domain of acos) but clamp it in
  // case in creeps outside due to rounding error
  x = std::clamp(x, static_cast<T>(-1), static_cast<T>(1));

  return acos(x);
}

// --- cout << Vector3 ------------------------------------------------------ //

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const Vector3<T>& v) {
  return out << '<' << v.x << ' ' << v.y << ' ' << v.z << '>';
}

// --- Vector4 -------------------------------------------------------------- //

template<typename T>
class Vector4 {
public:
  T x, y, z, w;

  // normalize homogeneous coordinates
  Vector3<T> divideByW() const {
    return Vector3<T> {x/w, y/w, z/w};
  }

  Vector3<T> dropW() const {
    return Vector3<T> {x, y, z};
  }
};

// --- Vector4 equality ----------------------------------------------------- //

// Invoking "T == T" may or may not be desired if T is floating-point. Use
// ApproxVector4 for approximate comparisons in Catch tests.

template<typename T>
inline bool operator==(const Vector4<T> &a, const Vector4<T> &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

template<typename T>
inline bool operator!=(const Vector4<T> &a, const Vector4<T> &b) {
  return !(a == b);
}

// --- Vector4 negation ----------------------------------------------------- //

template<typename T>
inline Vector4<T> operator-(const Vector4<T> &v) {
  return Vector4<T>{-v.x, -v.y, -v.z, -v.w};
}

// --- Vector4 + Vector4 ---------------------------------------------------- //

template<typename T>
inline Vector4<T> operator+(const Vector4<T> &a, const Vector4<T> &b) {
  return Vector4<T> {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

template<typename T>
inline Vector4<T>& operator+=(Vector4<T> &a, const Vector4<T> &b) {
  return a = a + b;
}

// --- Vector4 - Vector4 ---------------------------------------------------- //

template<typename T>
inline Vector4<T> operator-(const Vector4<T> &a, const Vector4<T> &b) {
  return Vector4<T> {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

template<typename T>
inline Vector4<T>& operator-=(Vector4<T> &a, const Vector4<T> &b) {
  return a = a - b;
}

// --- Vector4 * scalar ----------------------------------------------------- //

template<typename T>
inline Vector4<T> operator*(const Vector4<T> &v, T scalar) {
  return Vector4<T>{v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar};
}

template<typename T>
inline Vector4<T> operator*(T scalar, const Vector4<T> &v) {
  return Vector4<T>{scalar * v.x, scalar * v.y, scalar * v.z, scalar * v.w};
}

template<typename T>
inline Vector4<T>& operator*=(Vector4<T> &v, T scalar) {
  return v = v * scalar;
}

// --- Vector4 / scalar ----------------------------------------------------- //

template<typename T>
inline Vector4<T> operator/(const Vector4<T> &v, T scalar) {
  return Vector4<T>{v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar};
}

template<typename T>
inline Vector4<T>& operator/=(Vector4<T> &v, T scalar) {
  return v = v / scalar;
}

// --- Vector4 special functions ------------------------------------------- //

template<typename T>
inline T dot(const Vector4<T> &a, const Vector4<T> &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// --- cout << Vector4 ------------------------------------------------------ //

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const Vector4<T>& v) {
  return out << '<' << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w << '>';
}

#endif
