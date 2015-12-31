#ifndef MATH3D_H
#define MATH3D_H

#include <ostream>

// TODO *= etc.

class Mat4;
class Vec3;
class Vec4;
class Quat;
class Ray;

/*****************************************************************************
 * Mat4                                                                      *
 *****************************************************************************/

class Mat4 {
private:
  float mData[16]; // Column-major

public:
  static const Mat4 ZERO;
  static const Mat4 IDENTITY;

  static Mat4 translation(const Vec3 &v);
  static Mat4 rotation(float theta, const Vec3 &axis);
  static Mat4 rotation(const Quat &q);
  static Mat4 scale(const Vec3 &v);

  Mat4();
  Mat4(
    float r0c0, float r0c1, float r0c2, float r0c3,
    float r1c0, float r1c1, float r1c2, float r1c3,
    float r2c0, float r2c1, float r2c2, float r2c3,
    float r3c0, float r3c1, float r3c2, float r3c3
  );
  Mat4(const Mat4 &a);

  const float* data() const;
  float* data();
  float operator()(int row, int col) const;
  float& operator()(int row, int col);
  Mat4& operator=(const Mat4& a);
  Mat4 transpose() const;
};

Mat4 operator-(const Mat4 &a);
Mat4 operator+(const Mat4 &a, const Mat4 &b);
Mat4 operator-(const Mat4 &a, const Mat4 &b);
Mat4 operator*(float s, const Mat4 &a);
Mat4 operator*(const Mat4 &a, float s);
Mat4 operator*(const Mat4 &a, const Mat4 &b);
Mat4 operator/(const Mat4 &a, float d);
bool operator==(const Mat4 &a, const Mat4 &b);
bool operator!=(const Mat4 &a, const Mat4 &b);
std::ostream& operator<<(std::ostream& out, const Mat4& mat);

/*****************************************************************************
 * Vec3                                                                      *
 *****************************************************************************/

class Vec3 {
public:
  float x, y, z;

  static const Vec3 ZERO;
  static const Vec3 UNIT_X;
  static const Vec3 UNIT_Y;
  static const Vec3 UNIT_Z;

  Vec3();
  Vec3(float _x, float _y, float _z);
  Vec3(const Vec3 &a);

  Vec3& operator=(const Vec3& a);
  float len() const;
  float len2() const;
  Vec3 unit() const;
};

float len();
float len2();
Vec3 unit();
float dot(const Vec3 &a , const Vec3 &b);
Vec3 cross(const Vec3 &a, const Vec3 &b);
Vec3 proj(const Vec3 &a, const Vec3 &b); // projection of a onto b
float angleBetween(const Vec3 &a, const Vec3 &b); // in radians
Vec3 operator-(const Vec3 &v);
Vec3 operator+(const Vec3 &a, const Vec3 &b);
Vec3 operator-(const Vec3 &a, const Vec3 &b);
Vec3 operator*(const Vec3 &v, float s);
Vec3 operator*(float s, const Vec3 &v);
Vec3 operator/(const Vec3 &v, float d);
bool operator==(const Vec3 &a, const Vec3 &b);
bool operator!=(const Vec3 &a, const Vec3 &b);
std::ostream& operator<<(std::ostream& out, const Vec3& v);

/*****************************************************************************
 * Vec4                                                                      *
 *****************************************************************************/

class Vec4 {
public:
  float x, y, z, w;

  static const Vec4 ZERO;
  static const Vec4 UNIT_X;
  static const Vec4 UNIT_Y;
  static const Vec4 UNIT_Z;
  static const Vec4 UNIT_W;

  Vec4();
  Vec4(float _x, float _y, float _z, float _w);
  Vec4(const Vec4 &a);
  Vec4(const Vec3 &a, float _w);

  Vec3 unHomogenize() const; // Normalize homogeneous coordinates
};

Vec4 operator-(const Vec4 &v);
Vec4 operator+(const Vec4 &a, const Vec4 &b);
Vec4 operator-(const Vec4 &a, const Vec4 &b);
Vec4 operator*(const Vec4 &v, float s);
Vec4 operator*(float s, const Vec4 &v);
Vec4 operator/(const Vec4 &v, float d);
bool operator==(const Vec4 &a, const Vec4 &b);
bool operator!=(const Vec4 &a, const Vec4 &b);
std::ostream& operator<<(std::ostream& out, const Vec4& v);

/*****************************************************************************
 * Quat                                                                      *
 *****************************************************************************/

class Quat {
public:
  float r, x, y, z;

  static Quat rotation(float theta, const Vec3 &axis);

  Quat();
  Quat(float r_, float x_, float y_, float z_);
  Quat(const Quat &src);

  Quat unit() const; // a.k.a. versor
};

Quat operator*(const Quat &a, const Quat &b); // Hamilton product
std::ostream& operator<<(std::ostream& out, const Quat& q);

/*****************************************************************************
 * Misc                                                                      *
 *****************************************************************************/

class Ray {
public:
  Vec3 origin;
  Vec3 direction;

  Ray(const Vec3 &_origin, const Vec3 &_direction);
};

// Treats v as a column matrix and does m * v
Vec4 operator*(const Mat4 &m, const Vec4 &v);

// Transforms both origin and direction of a Ray
Ray operator*(const Mat4 &m, const Ray &r);

void assertMath();

#endif
