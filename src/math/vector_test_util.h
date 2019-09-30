#ifndef GLFUN_VECTOR_TEST_UTIL_H
#define GLFUN_VECTOR_TEST_UTIL_H

#include "math/vector.h"

#include "approx_container.h"

#include <cmath>

// --- Approx Vector3 ------------------------------------------------------- //

class ApproxVector3 : public ApproxContainer {
public:
  Vector3d vector;

  ApproxVector3() : vector{} {}
  ApproxVector3(Vector3d vector) : vector(vector) {}

  bool operator==(const Vector3<double> &v) const {
    return ApproxValue(vector.x) == v.x &&
           ApproxValue(vector.y) == v.y &&
           ApproxValue(vector.z) == v.z;
  }
};

inline bool operator==(const Vector3<double> &v, const ApproxVector3 &av) {
  return av == v;
}

inline bool operator!=(const ApproxVector3 &av, const Vector3<double> &v) {
  return !(av == v);
}

inline bool operator!=(const Vector3<double> &v, const ApproxVector3 &av) {
  return !(v == av);
}

inline std::ostream& operator<<(std::ostream& out, const ApproxVector3 &av) {
  return out << "Approx:\n" << av.vector;
}

// --- Approx Vector4 ------------------------------------------------------- //

class ApproxVector4 : public ApproxContainer {
public:
  Vector4d vector;

  ApproxVector4() : vector{} {}
  ApproxVector4(Vector4d vector) : vector(vector) {}

  bool operator==(const Vector4<double> &v) const {
    return ApproxValue(vector.x) == v.x &&
           ApproxValue(vector.y) == v.y &&
           ApproxValue(vector.z) == v.z &&
           ApproxValue(vector.w) == v.w;
  }
};

inline bool operator==(const Vector4<double> &v, const ApproxVector4 &av) {
  return av == v;
}

inline bool operator!=(const ApproxVector4 &av, const Vector4<double> &v) {
  return !(av == v);
}

inline bool operator!=(const Vector4<double> &v, const ApproxVector4 &av) {
  return !(v == av);
}

inline std::ostream& operator<<(std::ostream& out, const ApproxVector4 &av) {
  return out << "Approx:\n" << av.vector;
}

#endif
