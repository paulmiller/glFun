#ifndef GLFUN_MATH_MATRIX_FACTORIES_H
#define GLFUN_MATH_MATRIX_FACTORIES_H

#include "matrix.h"

#include <cassert>
#include <cmath>

template<typename T>
Matrix<T,3,3> MatrixFromColumnVectors(
  Vector3<T> c0, Vector3<T> c1, Vector3<T> c2
) {
  return Matrix<T,3,3> {{
    {c0.x, c1.x, c2.x},
    {c0.y, c1.y, c2.y},
    {c0.z, c1.z, c2.z}
  }};
}

template<typename T>
Matrix<T,4,4> MatrixFromColumnVectors(
  Vector4<T> c0, Vector4<T> c1, Vector4<T> c2, Vector4<T> c3
) {
  return Matrix<T,4,4> {{
    {c0.x, c1.x, c2.x, c3.x},
    {c0.y, c1.y, c2.y, c3.y},
    {c0.z, c1.z, c2.z, c3.z},
    {c0.w, c1.w, c2.w, c3.w}
  }};
}

template<typename T>
Matrix<T,4,4> TranslationMatrix4x4(Vector3<T> v) {
  return Matrix<T,4,4> {{
    {1, 0, 0, v.x},
    {0, 1, 0, v.y},
    {0, 0, 1, v.z},
    {0, 0, 0,   1}
  }};
}

template<typename T>
Matrix<T,4,4> RotationMatrix4x4(Vector3<T> axis, T angle) {
  // "axis" should be a unit vector. This is a very permissive assertion.
  assert(std::abs(axis.len2() - 1) < 0.1);

  T xy = axis.x * axis.y;
  T xz = axis.x * axis.z;
  T yz = axis.y * axis.z;

  T x2 = axis.x * axis.x;
  T y2 = axis.y * axis.y;
  T z2 = axis.z * axis.z;

  T sin_theta = sin(angle);
  T cos_theta = cos(angle);

  T x_sin_theta = axis.x * sin_theta;
  T y_sin_theta = axis.y * sin_theta;
  T z_sin_theta = axis.z * sin_theta;

  return Matrix<T,4,4> {{
    {
      cos_theta + x2 * (1 - cos_theta),
      xy * (1 - cos_theta) - z_sin_theta,
      xz * (1 - cos_theta) + y_sin_theta,
      0
    }, {
      xy * (1 - cos_theta) + z_sin_theta,
      cos_theta + y2 * (1 - cos_theta),
      yz * (1 - cos_theta) - x_sin_theta,
      0,
    }, {
      xz * (1 - cos_theta) - y_sin_theta,
      yz * (1 - cos_theta) + x_sin_theta,
      cos_theta + z2  * (1 - cos_theta),
      0,
    }, {
      0,
      0,
      0,
      1
    }
  }};
}

template<typename T1, typename T2,
  std::enable_if_t<std::is_convertible<T2,T1>::value, int> = 0>
Matrix<T1,4,4> RotationMatrix4x4(Vector3<T1> axis, T2 angle) {
  return RotationMatrix4x4(axis, static_cast<T1>(angle));
}

template<typename T>
Matrix<T,4,4> ScaleMatrix4x4(Vector3<T> v) {
  return Matrix<T,4,4> {{
    {v.x,   0,   0, 0},
    {  0, v.y,   0, 0},
    {  0,   0, v.z, 0},
    {  0,   0,   0, 1}
  }};
}

#endif
