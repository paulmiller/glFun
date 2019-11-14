#ifndef GLFUN_MATH_MATRIX_H
#define GLFUN_MATH_MATRIX_H

#include "vector.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <ostream>
#include <type_traits>

/*
A template for arbitrary types and sizes of matrices

Elements are in row-major order (unlike OpenGL) to allow for pretty aggregate
initialization of matrices. For example:

  matrix<int,2,2> m{{
    {1, 2},
    {3, 4}
  }};

will create the matrix:

  [ 1 2 ]
  [ 3 4 ]

as opposed to:

  [ 1 3 ]
  [ 2 4 ]

And:

  matrix<int,2,2> m {};

will create:

  [ 0 0 ]
  [ 0 0 ]

Whereas:

  matrix<int,2,2> m;

will have undefined contents.
*/
template<typename T, int Rows, int Cols> class Matrix;

typedef Matrix<float , 3, 3> Matrix3x3f;
typedef Matrix<double, 3, 3> Matrix3x3d;
typedef Matrix<float , 4, 4> Matrix4x4f;
typedef Matrix<double, 4, 4> Matrix4x4d;

extern const Matrix3x3f Zero_Matrix3x3f;
extern const Matrix3x3f Identity_Matrix3x3f;
extern const Matrix3x3d Zero_Matrix3x3d;
extern const Matrix3x3d Identity_Matrix3x3d;
extern const Matrix4x4f Zero_Matrix4x4f;
extern const Matrix4x4f Identity_Matrix4x4f;
extern const Matrix4x4d Zero_Matrix4x4d;
extern const Matrix4x4d Identity_Matrix4x4d;

template<typename T, int Rows, int Cols>
class Matrix {
  static_assert(Rows >= 1);
  static_assert(Cols >= 1);

public:
  T data[Rows][Cols];

  T& operator()(int row, int col) {
    assert(0 <= row); assert(row < Rows);
    assert(0 <= col); assert(col < Cols);
    return data[row][col];
  }

  T operator()(int row, int col) const {
    return const_cast<Matrix<T,Rows,Cols>*>(this)->operator()(row, col);
  }

  Matrix<T, Cols, Rows> transpose() const {
    Matrix<T, Cols, Rows> trans;
    for(int r = 0; r < Rows; r++) {
      for(int c = 0; c < Cols; c++) {
        trans.data[c][r] = data[r][c];
      }
    }
    return trans;
  }

  template<typename T_ = T, int Rows_ = Rows, int Cols_ = Cols,
    std::enable_if_t<Rows_ == 1 && Cols_ == 1, int> = 0>
  T_ determinant() const {
    return data[0][0];
  }

  template<typename T_ = T, int Rows_ = Rows, int Cols_ = Cols,
    std::enable_if_t<Rows_ == 2 && Cols_ == 2, int> = 0>
  T_ determinant() const {
    return data[0][0] * data[1][1] - data[0][1] * data[1][0];
  }

  template<typename T_ = T, int Rows_ = Rows, int Cols_ = Cols,
    std::enable_if_t<Rows_ == 3 && Cols_ == 3, int> = 0>
  T_ determinant() const {
    return data[0][0] * data[1][1] * data[2][2]
         + data[0][1] * data[1][2] * data[2][0]
         + data[0][2] * data[1][0] * data[2][1]
         - data[0][0] * data[1][2] * data[2][1]
         - data[0][1] * data[1][0] * data[2][2]
         - data[0][2] * data[1][1] * data[2][0];
  }

  bool isfinite() const {
    for(int r = 0; r < Rows; r++) {
      for(int c = 0; c < Cols; c++) {
        if(!std::isfinite(data[r][c]))
          return false;
      }
    }
    return true;
  }
};

// --- matrix equality ------------------------------------------------------ //

// Invoking "T != T" may or may not be desired if T is floating-point. Use
// ApproxMatrix for approximate comparisons in Catch tests.

template<typename T, int Rows, int Cols>
inline bool operator==(
  const Matrix<T, Rows, Cols> &left,
  const Matrix<T, Rows, Cols> &right
) {
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      if(left.data[r][c] != right.data[r][c])
        return false;
    }
  }
  return true;
}

template<typename T, int Rows, int Cols>
inline bool operator!=(
  const Matrix<T, Rows, Cols> &left,
  const Matrix<T, Rows, Cols> &right
) {
  return !(left == right);
}

// --- matrix negation ------------------------------------------------------ //

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator-(const Matrix<T, Rows, Cols> &m) {
  Matrix<T, Rows, Cols> neg;
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      neg.data[r][c] = -m.data[r][c];
    }
  }
  return neg;
}

// --- matrix + matrix ------------------------------------------------------ //

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator+(
  const Matrix<T, Rows, Cols> &left,
  const Matrix<T, Rows, Cols> &right
) {
  Matrix<T, Rows, Cols> sum {};
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      sum.data[r][c] = left.data[r][c] + right.data[r][c];
    }
  }
  return sum;
}

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols>& operator+=(
  Matrix<T, Rows, Cols> &left,
  const Matrix<T, Rows, Cols> &right
) {
  left = left + right;
  return left;
}

// --- matrix - matrix ------------------------------------------------------ //

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator-(
  const Matrix<T, Rows, Cols> &left,
  const Matrix<T, Rows, Cols> &right
) {
  Matrix<T, Rows, Cols> difference;
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      difference.data[r][c] = left.data[r][c] - right.data[r][c];
    }
  }
  return difference;
}

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols>& operator-=(
  Matrix<T, Rows, Cols> &left,
  const Matrix<T, Rows, Cols> &right
) {
  left = left - right;
  return left;
}

// --- matrix * scalar ------------------------------------------------------ //

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator*(
  T scalar, const Matrix<T, Rows, Cols> &m
) {
  Matrix<T, Rows, Cols> product;
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      product.data[r][c] = scalar * m.data[r][c];
    }
  }
  return product;
}

template<typename Matrix_T, typename Scalar_T, int Rows, int Cols,
  std::enable_if_t<std::is_convertible<Scalar_T,Matrix_T>::value, int> = 0>
inline Matrix<Matrix_T, Rows, Cols> operator*(
  Scalar_T scalar,
  const Matrix<Matrix_T, Rows, Cols> &m
) {
  return static_cast<Matrix_T>(scalar) * m;
}

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator*(
  const Matrix<T, Rows, Cols> &m, T scalar
) {
  return scalar * m;
}

template<typename Matrix_T, typename Scalar_T, int Rows, int Cols,
  std::enable_if_t<std::is_convertible<Scalar_T,Matrix_T>::value, int> = 0>
inline Matrix<Matrix_T, Rows, Cols> operator*(
  const Matrix<Matrix_T, Rows, Cols> &m,
  Scalar_T scalar
) {
  return m * static_cast<Matrix_T>(scalar);
}

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols>& operator*=(Matrix<T, Rows, Cols> &m, T scalar) {
  m = m * scalar;
  return m;
}

template<typename Matrix_T, typename Scalar_T, int Rows, int Cols,
  std::enable_if_t<std::is_convertible<Scalar_T,Matrix_T>::value, int> = 0>
inline Matrix<Matrix_T, Rows, Cols>& operator*=(
  Matrix<Matrix_T, Rows, Cols> &m,
  Scalar_T scalar
) {
  return m *= static_cast<Matrix_T>(scalar);
}

// --- matrix * matrix ------------------------------------------------------ //

template<typename T, int LeftRows, int LeftCols, int RightCols>
inline Matrix<T, LeftRows, RightCols> operator*(
  const Matrix<T, LeftRows, LeftCols> &left,
  const Matrix<T, LeftCols, RightCols> &right
) {
  Matrix<T, LeftRows, RightCols> product {};
  for(int r = 0; r < LeftRows; r++) {
    for(int c = 0; c < RightCols; c++) {
      for(int i = 0; i < LeftCols; i++) {
        product.data[r][c] += left.data[r][i] * right.data[i][c];
      }
    }
  }
  return product;
}

// A *= B requires that A and B have the name number of columns, or the result
// of A * B won't fit in A.
template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator*=(
  Matrix<T, Rows, Cols> &left,
  const Matrix<T, Cols, Cols> &right
) {
  left = left * right;
  return left;
}

// --- matrix / scalar ------------------------------------------------------ //

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols> operator/(
  const Matrix<T, Rows, Cols> &m, T scalar
) {
  Matrix<T, Rows, Cols> quotient;
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      quotient.data[r][c] = m.data[r][c] / scalar;
    }
  }
  return quotient;
}

template<typename Matrix_T, typename Scalar_T, int Rows, int Cols,
  std::enable_if_t<std::is_convertible<Scalar_T,Matrix_T>::value, int> = 0>
inline Matrix<Matrix_T, Rows, Cols> operator/(
  const Matrix<Matrix_T, Rows, Cols> &m,
  Scalar_T scalar
) {
  return m / static_cast<Matrix_T>(scalar);
}

template<typename T, int Rows, int Cols>
inline Matrix<T, Rows, Cols>& operator/=(Matrix<T, Rows, Cols> &m, T scalar) {
  m = m / scalar;
  return m;
}

template<typename Matrix_T, typename Scalar_T, int Rows, int Cols,
  std::enable_if_t<std::is_convertible<Scalar_T,Matrix_T>::value, int> = 0>
inline Matrix<Matrix_T, Rows, Cols>& operator/=(
  Matrix<Matrix_T, Rows, Cols> &m,
  Scalar_T scalar
) {
  return m /= static_cast<Matrix_T>(scalar);
}

// --- cout << matrix ------------------------------------------------------- //

template<typename T, int Rows, int Cols>
inline std::ostream& operator<<(
  std::ostream& out, Matrix<T, Rows, Cols> matrix
) {
  for(int r = 0; r < Rows; r++) {
    out << '[';
    for(int c = 0; c < Cols; c++)
      out << ' ' << matrix(r, c);
    out << " ]\n";
  }
  return out;
}

#endif
