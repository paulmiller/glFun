#ifndef MATH3D_H
#define MATH3D_H

#include <ostream>
#include <string>

// all angles in radians

class Mat3;
class Mat4;
class Vec2;
class Vec3;
class Vec4;
class Quat;
class Ray;

/*
class Vec2 {
public:
  float x, y;

  static const Vec2 ZERO;
  static const Vec2 UNIT_X;
  static const Vec2 UNIT_Y;

  Vec2();
  Vec2(float x, float y);
  Vec2(const Vec2 &a);

  Vec2& operator=(const Vec2& a);
  Vec2& operator+=(const Vec2 &b);
  Vec2& operator-=(const Vec2 &b);
  Vec2& operator*=(float s);
  float len() const;
  float len2() const;
  Vec2 unit() const;
};

float dot(const Vec2 &a , const Vec2 &b);
Vec2 proj(const Vec2 &a, const Vec2 &b); // projection of a onto b
float angleBetween(const Vec2 &a, const Vec2 &b);
Vec2 operator-(const Vec2 &v);
Vec2 operator+(const Vec2 &a, const Vec2 &b);
Vec2 operator-(const Vec2 &a, const Vec2 &b);
Vec2 operator*(const Vec2 &v, float s);
Vec2 operator*(float s, const Vec2 &v);
Vec2 operator/(const Vec2 &v, float d);
bool operator==(const Vec2 &a, const Vec2 &b);
bool operator!=(const Vec2 &a, const Vec2 &b);
std::ostream& operator<<(std::ostream& out, const Vec2& v);
*/

/*****************************************************************************
 * Quat                                                                      *
 *****************************************************************************/

class Quat {
public:
  static const Quat IDENTITY;

  float r, x, y, z;

  static Quat rotation(const Vec3 &axis, float angle);

  Quat();
  Quat(float r, float x, float y, float z);
  Quat(const Quat &src);

  Quat& operator=(const Quat& a);
  Quat& operator*=(const Quat& b);

  Quat unit() const; // a.k.a. versor
};

Quat operator*(const Quat &a, const Quat &b); // Hamilton product
bool operator==(const Quat &a, const Quat &b);
bool operator!=(const Quat &a, const Quat &b);
std::ostream& operator<<(std::ostream& out, const Quat& q);

/*****************************************************************************
 * Misc                                                                      *
 *****************************************************************************/

// Determinant of the 3x3 matrix with columns [A B C]
float det(const Vec3 &A, const Vec3 &B, const Vec3 &C);

class Ray {
public:
  Vec3 origin;
  Vec3 direction;

  Ray(const Vec3 &origin, const Vec3 &direction);
};

// Transforms both origin and direction of a Ray
Ray operator*(const Mat4 &m, const Ray &r);

void assertMath();

#endif
