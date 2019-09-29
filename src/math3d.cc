#include "math3d.h"

#include "util.h"

#include <cmath>
#include <cassert>

/*****************************************************************************
 * Mat3                                                                      *
 *****************************************************************************/

/*
Mat4 Mat4::translation(const Vec3 &v) {
  return Mat4(
    1, 0, 0, v.x,
    0, 1, 0, v.y,
    0, 0, 1, v.z,
    0, 0, 0, 1
  );
}

Mat4 Mat4::rotation(const Vec3 &axis, float angle) {
  float xy = axis.x * axis.y;
  float xz = axis.x * axis.z;
  float yz = axis.y * axis.z;

  float x2 = axis.x * axis.x;
  float y2 = axis.y * axis.y;
  float z2 = axis.z * axis.z;

  float sin_theta = sin(angle);
  float cos_theta = cos(angle);

  float x_sin_theta = axis.x * sin_theta;
  float y_sin_theta = axis.y * sin_theta;
  float z_sin_theta = axis.z * sin_theta;

  return Mat4(
    cos_theta + x2 * (1 - cos_theta),
    xy * (1 - cos_theta) - z_sin_theta,
    xz * (1 - cos_theta) + y_sin_theta,
    0,

    xy * (1 - cos_theta) + z_sin_theta,
    cos_theta + y2 * (1 - cos_theta),
    yz * (1 - cos_theta) - x_sin_theta,
    0,

    xz * (1 - cos_theta) - y_sin_theta,
    yz * (1 - cos_theta) + x_sin_theta,
    cos_theta + z2  * (1 - cos_theta),
    0,

    0,
    0,
    0,
    1
  );
}

Mat4 Mat4::rotation(const Quat &q) {
  Quat u = q.unit();

  return Mat4(
    1-2*(u.y*u.y+u.z*u.z),   2*(u.x*u.y-u.r*u.z),   2*(u.x*u.z+u.r*u.y), 0,
      2*(u.x*u.y+u.r*u.z), 1-2*(u.x*u.x+u.z*u.z),   2*(u.y*u.z-u.r*u.x), 0,
      2*(u.x*u.z-u.r*u.y),   2*(u.y*u.z+u.r*u.x), 1-2*(u.x*u.x+u.y*u.y), 0,
                         0,                    0,                     0, 1
  );
}

Mat4 Mat4::scale(const Vec3 &v) {
  return Mat4(
    v.x,   0,   0, 0,
      0, v.y,   0, 0,
      0,   0, v.z, 0,
      0,   0,   0, 1
  );
}
*/

/*****************************************************************************
 * Vec2                                                                      *
 *****************************************************************************/

const Vec2 Vec2::ZERO(0, 0);
const Vec2 Vec2::UNIT_X(1, 0);
const Vec2 Vec2::UNIT_Y(0, 1);

Vec2::Vec2() : Vec2(ZERO) {} // TODO initialization order bugs?
Vec2::Vec2(float x_, float y_) : x(x_), y(y_) {}
Vec2::Vec2(const Vec2 &a) : x(a.x), y(a.y) {};

Vec2& Vec2::operator=(const Vec2& a) {
  x = a.x;
  y = a.y;
  return *this;
}

Vec2& Vec2::operator+=(const Vec2 &b) {
  x += b.x;
  y += b.y;
  return *this;
}

Vec2& Vec2::operator-=(const Vec2 &b) {
  x -= b.x;
  y -= b.y;
  return *this;
}

Vec2& Vec2::operator*=(float s) {
  x *= s;
  y *= s;
  return *this;
}

float Vec2::len() const {
  return sqrt(len2());
}

float Vec2::len2() const {
  return x*x + y*y;
}

Vec2 Vec2::unit() const {
  return (*this) * invSqrt(len2());
}

float dot(const Vec2 &a , const Vec2 &b) {
  return a.x * b.x + a.y * b.y;
}

Vec2 proj(const Vec2 &a, const Vec2 &b) {
  Vec2 b1 = b.unit();
  return dot(a, b1) * b1;
}

float angleBetween(const Vec2 &a, const Vec2 &b) {
  return acos( dot(a, b) / (a.len() * b.len()) );
}

Vec2 operator-(const Vec2 &v) {
  return Vec2(-v.x, -v.y);
}

Vec2 operator+(const Vec2 &a, const Vec2 &b) {
  return Vec2(a.x + b.x, a.y + b.y);
}

Vec2 operator-(const Vec2 &a, const Vec2 &b) {
  return Vec2(a.x - b.x, a.y - b.y);
}

Vec2 operator*(const Vec2 &v, float s) {
  return Vec2(v.x * s, v.y * s);
}

Vec2 operator*(float s, const Vec2 &v) {
  return Vec2(s * v.x, s * v.y);
}

Vec2 operator/(const Vec2 &v, float d) {
  return Vec2(v.x / d, v.y / d);
}

bool operator==(const Vec2 &a, const Vec2 &b) {
  return a.x == b.x && a.y == b.y;
}

bool operator!=(const Vec2 &a, const Vec2 &b) {
  return a.x != b.x || a.y != b.y;
}

std::ostream& operator<<(std::ostream& out, const Vec2& v) {
  out << '<' << v.x << ' ' << v.y << '>';
  return out;
}

/*****************************************************************************
 * Vec3                                                                      *
 *****************************************************************************/

const Vec3 Vec3::ZERO(0, 0, 0);
const Vec3 Vec3::UNIT_X(1, 0, 0);
const Vec3 Vec3::UNIT_Y(0, 1, 0);
const Vec3 Vec3::UNIT_Z(0, 0, 1);

Vec3::Vec3() : Vec3(ZERO) {} // TODO initialization order bugs?
Vec3::Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
Vec3::Vec3(const Vec2 &xy, float z_) : x(xy.x), y(xy.y), z(z_) {}
Vec3::Vec3(const Vec3 &a) : x(a.x), y(a.y), z(a.z) {};

Vec3& Vec3::operator=(const Vec3& a) {
  x = a.x;
  y = a.y;
  z = a.z;
  return *this;
}

Vec3& Vec3::operator+=(const Vec3 &b) {
  x += b.x;
  y += b.y;
  z += b.z;
  return *this;
}

Vec3& Vec3::operator-=(const Vec3 &b) {
  x -= b.x;
  y -= b.y;
  z -= b.z;
  return *this;
}

Vec3& Vec3::operator*=(float s) {
  x *= s;
  y *= s;
  z *= s;
  return *this;
}

float Vec3::len() const {
  return sqrt(len2());
}

float Vec3::len2() const {
  return x*x + y*y + z*z;
}

Vec3 Vec3::unit() const {
  return (*this) * invSqrt(len2());
}

float dot(const Vec3 &a , const Vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(const Vec3 &a, const Vec3 &b) {
  return Vec3(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  );
}

Vec3 proj(const Vec3 &a, const Vec3 &b) {
  Vec3 b1 = b.unit();
  return dot(a, b1) * b1;
}

float angleBetween(const Vec3 &a, const Vec3 &b) {
  return acos( dot(a, b) / (a.len() * b.len()) );
}

Quat quatBetween(const Vec3 &a, const Vec3 &b) {
  return Quat::rotation(cross(a, b), angleBetween(a, b));
}

Vec3 operator-(const Vec3 &v) {
  return Vec3(-v.x, -v.y, -v.z);
}

Vec3 operator+(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 operator*(const Vec3 &v, float s) {
  return Vec3(v.x * s, v.y * s, v.z * s);
}

Vec3 operator*(float s, const Vec3 &v) {
  return Vec3(s * v.x, s * v.y, s * v.z);
}

Vec3 operator/(const Vec3 &v, float d) {
  return Vec3(v.x / d, v.y / d, v.z / d);
}

bool operator==(const Vec3 &a, const Vec3 &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!=(const Vec3 &a, const Vec3 &b) {
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

std::ostream& operator<<(std::ostream& out, const Vec3& v) {
  out << '<' << v.x << ' ' << v.y << ' ' << v.z << '>';
  return out;
}

/*****************************************************************************
 * Vec4                                                                      *
 *****************************************************************************/

const Vec4 Vec4::ZERO(0, 0, 0, 0);
const Vec4 Vec4::UNIT_X(1, 0, 0, 0);
const Vec4 Vec4::UNIT_Y(0, 1, 0, 0);
const Vec4 Vec4::UNIT_Z(0, 0, 1, 0);
const Vec4 Vec4::UNIT_W(0, 0, 0, 1);

Vec4::Vec4() : Vec4(ZERO) {} // TODO initialization order bugs?
Vec4::Vec4(float x_, float y_, float z_, float w_)
    : x(x_), y(y_), z(z_), w(w_) {}
Vec4::Vec4(const Vec3 &xyz, float w_) : x(xyz.x), y(xyz.y), z(xyz.z), w(w_) {}
Vec4::Vec4(const Vec4 &a) : x(a.x), y(a.y), z(a.z), w(a.w) {}

Vec4& Vec4::operator=(const Vec4& a) {
  x = a.x;
  y = a.y;
  z = a.z;
  w = a.w;
  return *this;
}

Vec4& Vec4::operator+=(const Vec4 &b) {
  x += b.x;
  y += b.y;
  z += b.z;
  w += b.w;
  return *this;
}

Vec4& Vec4::operator-=(const Vec4 &b) {
  x -= b.x;
  y -= b.y;
  z -= b.z;
  w -= b.w;
  return *this;
}

Vec4& Vec4::operator*=(float s) {
  x *= s;
  y *= s;
  z *= s;
  w *= s;
  return *this;
}

Vec3 Vec4::unHomogenize() const {
  return Vec3(x/w, y/w, z/w);
}

Vec3 Vec4::dropW() const {
  return Vec3(x, y, z);
}

Vec4 operator-(const Vec4 &v) {
  return Vec4(-v.x, -v.y, -v.z, -v.w);
}

Vec4 operator+(const Vec4 &a, const Vec4 &b) {
  return Vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Vec4 operator-(const Vec4 &a, const Vec4 &b) {
  return Vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Vec4 operator*(const Vec4 &v, float s) {
  return Vec4(v.x * s, v.y * s, v.z * s, v.w * s);
}

Vec4 operator*(float s, const Vec4 &v) {
  return Vec4(s * v.x, s * v.y, s * v.z, s * v.w);
}

Vec4 operator/(const Vec4 &v, float d) {
  return Vec4(v.x / d, v.y / d, v.z / d, v.w / d);
}

bool operator==(const Vec4 &a, const Vec4 &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool operator!=(const Vec4 &a, const Vec4 &b) {
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

std::ostream& operator<<(std::ostream& out, const Vec4& v) {
  out << '<' << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w << '>';
  return out;
}

/*****************************************************************************
 * Quat                                                                      *
 *****************************************************************************/

const Quat Quat::IDENTITY(1, 0, 0, 0);

Quat Quat::rotation(const Vec3 &axis, float angle) {
  Vec3 u = axis.unit();
  float r = cos(angle / 2);
  float i = sin(angle / 2);
  return Quat(r, i*u.x, i*u.y, i*u.z);
}

Quat::Quat() {}
Quat::Quat(float r_, float x_, float y_, float z_) :
  r(r_), x(x_), y(y_), z(z_) {}
Quat::Quat(const Quat &src) :
  r(src.r), x(src.x), y(src.y), z(src.z) {}

Quat Quat::unit() const {
  float s = invSqrt(r*r + x*x + y*y + z*z);
  return Quat(s*r, s*x, s*y, s*z);
}

Quat& Quat::operator=(const Quat& a) {
  r = a.r;
  x = a.x;
  y = a.y;
  z = a.z;
  return *this;
}

Quat& Quat::operator*=(const Quat& b) {
  return (*this) = (*this) * b;
}

Quat operator*(const Quat &a, const Quat &b) {
  return Quat(
    a.r*b.r - a.x*b.x - a.y*b.y - a.z*b.z,
    a.r*b.x + a.x*b.r + a.y*b.z - a.z*b.y,
    a.r*b.y - a.x*b.z + a.y*b.r + a.z*b.x,
    a.r*b.z + a.x*b.y - a.y*b.x + a.z*b.r
  );
}

bool operator==(const Quat &a, const Quat &b) {
  return a.r == b.r && a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!=(const Quat &a, const Quat &b) {
  return a.r != b.r || a.x != b.x || a.y != b.y || a.z != b.z;
}

std::ostream& operator<<(std::ostream& out, const Quat& q) {
  out << q.r << '+' << q.x << "i+" << q.y << "j+" << q.z << 'k';
  return out;
}

/*****************************************************************************
 * Misc                                                                      *
 *****************************************************************************/

// Determinant of the 3x3 matrix with columns [A B C]
float det(const Vec3 &A, const Vec3 &B, const Vec3 &C) {
  return A.x * B.y * C.z + B.x * C.y * A.z + C.x * A.y * B.z
       - C.x * B.y * A.z - B.x * A.y * C.z - A.x * C.y * B.z;
}

Ray::Ray(const Vec3 &origin_, const Vec3 &direction_) :
  origin(origin_), direction(direction_) {}

Ray operator*(const Mat4 &m, const Ray &r) {
  Vec4 o = m * Vec4(r.origin, 1);
  Vec4 d = m * Vec4(r.direction, 0);
  return Ray(o.unHomogenize(), d.dropW());
}
