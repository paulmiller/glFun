#ifndef GLFUN_MATRIX_TEST_UTIL_H
#define GLFUN_MATRIX_TEST_UTIL_H

#include "math/matrix.h"

#include "approx_container.h"

#include <cmath>

template<int Rows, int Cols>
class ApproxMatrix : public ApproxContainer {
public:
  Matrix<double, Rows, Cols> matrix;

  ApproxMatrix() : matrix{} {}
  ApproxMatrix(Matrix<double, Rows, Cols> matrix) : matrix(matrix) {}

  bool operator==(const Matrix<double, Rows, Cols> &m) const {
    for(int r = 0; r < Rows; r++) {
      for(int c = 0; c < Cols; c++) {
        if(ApproxValue(matrix.data[r][c]) != m.data[r][c])
          return false;
      }
    }
    return true;
  }
};

template<int Rows, int Cols>
inline bool operator==(
  const Matrix<double, Rows, Cols> &m,
  const ApproxMatrix<Rows, Cols> &am
) {
  return am == m;
}

template<int Rows, int Cols>
inline bool operator!=(
  const ApproxMatrix<Rows, Cols> &am,
  const Matrix<double, Rows, Cols> &m
) {
  return !(am == m);
}

template<int Rows, int Cols>
inline bool operator!=(
  const Matrix<double, Rows, Cols> &m,
  const ApproxMatrix<Rows, Cols> &am
) {
  return !(m == am);
}

template<int Rows, int Cols>
inline std::ostream& operator<<(
  std::ostream& out, ApproxMatrix<Rows, Cols> am
) {
  return out << "Approx:\n" << am.matrix;
}

#endif
