#ifndef GLFUN_MATH_MATRIX_VECTOR_PRODUCT_H
#define GLFUN_MATH_MATRIX_VECTOR_PRODUCT_H

#include "math/matrix.h"
#include "math/vector.h"

template<typename T>
inline Vector3<T> operator*(
  const Matrix<T,3,3> &m,
  const Vector3<T> &v
) {
  return Vector3<T> {
    m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z,
    m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z,
    m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z
  };
}

template<typename T>
inline Vector4<T> operator*(
  const Matrix<T,4,4> &m,
  const Vector4<T> &v
) {
  return Vector4<T> {
    m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z + m(0,3) * v.w,
    m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z + m(1,3) * v.w,
    m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z + m(2,3) * v.w,
    m(3,0) * v.x + m(3,1) * v.y + m(3,2) * v.z + m(3,3) * v.w
  };
}

#endif
