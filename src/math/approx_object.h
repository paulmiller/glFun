#ifndef GLFUN_APPROX_OBJECT_H
#define GLFUN_APPROX_OBJECT_H

#include "catch.h"

#include "matrix.h"
#include "vector.h"

#include <limits>

// ApproxObject wraps a Vector or Matrix to support approximate comparisons

template<typename T> class ApproxObject;
using ApproxVector3d = ApproxObject<Vector3d>;
using ApproxVector4d = ApproxObject<Vector4d>;
using ApproxMatrix3x3d = ApproxObject<Matrix3x3d>;
using ApproxMatrix4x4d = ApproxObject<Matrix4x4d>;

template<typename T>
class ApproxObject {
public:
  T value;

  ApproxObject(const T &value) : value(value) {}

  ApproxObject &epsilon(double e) {
    epsilon_ = e;
    return *this;
  }

  ApproxObject &margin(double m) {
    margin_ = m;
    return *this;
  }

  ApproxObject &scale(double s) {
    scale_ = s;
    return *this;
  }

  Approx MakeApprox(double d) const {
    Approx a = Approx(d);
    if(!std::isnan(epsilon_))
      a.epsilon(epsilon_);
    if(!std::isnan(margin_))
      a.margin(margin_);
    if(!std::isnan(scale_))
      a.scale(scale_);
    return a;
  }

private:
  static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

  // if not NaN, use for the the corresponding Approx parameter
  double epsilon_ = NaN;
  double margin_  = NaN;
  double scale_   = NaN;
};

inline bool operator==(const ApproxVector3d &left, const Vector3d &right) {
  return left.MakeApprox(left.value.x) == right.x &&
         left.MakeApprox(left.value.y) == right.y &&
         left.MakeApprox(left.value.z) == right.z;
}

inline bool operator==(const ApproxVector4d &left, const Vector4d &right) {
  return left.MakeApprox(left.value.x) == right.x &&
         left.MakeApprox(left.value.y) == right.y &&
         left.MakeApprox(left.value.z) == right.z &&
         left.MakeApprox(left.value.w) == right.w;
}

template<int Rows, int Cols>
inline bool operator==(
  const ApproxObject<Matrix<double,Rows,Cols>> &left,
  const Matrix<double,Rows,Cols> &right
) {
  for(int r = 0; r < Rows; r++) {
    for(int c = 0; c < Cols; c++) {
      if(left.MakeApprox(left.value.data[r][c]) != right.data[r][c])
        return false;
    }
  }
  return true;
}

template<typename T>
inline bool operator==(const T &left, const ApproxObject<T> &right) {
  return right == left;
}

template<typename T>
inline bool operator!=(const ApproxObject<T> &left, const T &right) {
  return !(left == right);
}

template<typename T>
inline bool operator!=(const T &left, const ApproxObject<T> &right) {
  return !(left == right);
}

#endif
