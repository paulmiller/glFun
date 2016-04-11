#include "math3d.h"

#include "util.h"

#include <cmath>
#include <cassert>

/*****************************************************************************
 * Mat4                                                                      *
 *****************************************************************************/

const Mat4 Mat4::ZERO(
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
);

const Mat4 Mat4::IDENTITY(
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1
);

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

  float sinTheta = sin(angle);
  float cosTheta = cos(angle);

  float xSinTheta = axis.x * sinTheta;
  float ySinTheta = axis.y * sinTheta;
  float zSinTheta = axis.z * sinTheta;

  return Mat4(
    cosTheta + x2 * (1 - cosTheta),
    xy * (1 - cosTheta) - zSinTheta,
    xz * (1 - cosTheta) + ySinTheta,
    0,

    xy * (1 - cosTheta) + zSinTheta,
    cosTheta + y2 * (1 - cosTheta),
    yz * (1 - cosTheta) - xSinTheta,
    0,

    xz * (1 - cosTheta) - ySinTheta,
    yz * (1 - cosTheta) + xSinTheta,
    cosTheta + z2  * (1 - cosTheta),
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

Mat4::Mat4() {}

Mat4::Mat4(
  float r0c0, float r0c1, float r0c2, float r0c3,
  float r1c0, float r1c1, float r1c2, float r1c3,
  float r2c0, float r2c1, float r2c2, float r2c3,
  float r3c0, float r3c1, float r3c2, float r3c3
) : mData{
  r0c0, r1c0, r2c0, r3c0,
  r0c1, r1c1, r2c1, r3c1,
  r0c2, r1c2, r2c2, r3c2,
  r0c3, r1c3, r2c3, r3c3
} {}

Mat4::Mat4(const Mat4& a) : mData{
  a.mData[ 0], a.mData[ 1], a.mData[ 2], a.mData[ 3],
  a.mData[ 4], a.mData[ 5], a.mData[ 6], a.mData[ 7],
  a.mData[ 8], a.mData[ 9], a.mData[10], a.mData[11],
  a.mData[12], a.mData[13], a.mData[14], a.mData[15]
} {}

const float* Mat4::data() const {
  return mData;
}

float* Mat4::data() {
  return mData;
}

float Mat4::operator()(int row, int col) const {
  assert(0 <= row && row <= 3);
  assert(0 <= col && col <= 3);
  return mData[4 * col + row];
}

float& Mat4::operator()(int row, int col) {
  assert(0 <= row && row <= 3);
  assert(0 <= col && col <= 3);
  return mData[4 * col + row];
}

Mat4& Mat4::operator=(const Mat4& a) {
  for(int i = 0; i < 16; i++) {
    mData[i] = a.mData[i];
  }
  return *this;
}

Mat4 Mat4::transpose() const {
  const Mat4 &a = *this;
  return Mat4(
    a(0,0), a(1,0), a(2,0), a(3,0),
    a(0,1), a(1,1), a(2,1), a(3,1),
    a(0,2), a(1,2), a(2,2), a(3,2),
    a(0,3), a(1,3), a(2,3), a(3,3)
  );
}

Mat4 operator-(const Mat4 &a) {
  return Mat4(
    -a(0,0), -a(0,1), -a(0,2), -a(0,3),
    -a(1,0), -a(1,1), -a(1,2), -a(1,3),
    -a(2,0), -a(2,1), -a(2,2), -a(2,3),
    -a(3,0), -a(3,1), -a(3,2), -a(3,3)
  );
}

Mat4 operator+(const Mat4 &a, const Mat4 &b) {
  return Mat4(
    a(0,0) + b(0,0), a(0,1) + b(0,1), a(0,2) + b(0,2), a(0,3) + b(0,3),
    a(1,0) + b(1,0), a(1,1) + b(1,1), a(1,2) + b(1,2), a(1,3) + b(1,3),
    a(2,0) + b(2,0), a(2,1) + b(2,1), a(2,2) + b(2,2), a(2,3) + b(2,3),
    a(3,0) + b(3,0), a(3,1) + b(3,1), a(3,2) + b(3,2), a(3,3) + b(3,3)
  );
}

Mat4 operator-(const Mat4 &a, const Mat4 &b) {
  return Mat4(
    a(0,0) - b(0,0), a(0,1) - b(0,1), a(0,2) - b(0,2), a(0,3) - b(0,3),
    a(1,0) - b(1,0), a(1,1) - b(1,1), a(1,2) - b(1,2), a(1,3) - b(1,3),
    a(2,0) - b(2,0), a(2,1) - b(2,1), a(2,2) - b(2,2), a(2,3) - b(2,3),
    a(3,0) - b(3,0), a(3,1) - b(3,1), a(3,2) - b(3,2), a(3,3) - b(3,3)
  );
}

Mat4 operator*(float s, const Mat4 &a) {
  return Mat4(
    s * a(0,0), s * a(0,1), s * a(0,2), s * a(0,3),
    s * a(1,0), s * a(1,1), s * a(1,2), s * a(1,3),
    s * a(2,0), s * a(2,1), s * a(2,2), s * a(2,3),
    s * a(3,0), s * a(3,1), s * a(3,2), s * a(3,3)
  );
}

Mat4 operator*(const Mat4 &a, float s) {
  return Mat4(
    a(0,0) * s, a(0,1) * s, a(0,2) * s, a(0,3) * s,
    a(1,0) * s, a(1,1) * s, a(1,2) * s, a(1,3) * s,
    a(2,0) * s, a(2,1) * s, a(2,2) * s, a(2,3) * s,
    a(3,0) * s, a(3,1) * s, a(3,2) * s, a(3,3) * s
  );
}

Mat4 operator*(const Mat4 &a, const Mat4 &b) {
  Mat4 p;
  for(int r = 0; r < 4; r++) {
    for(int c = 0; c < 4; c++) {
      p(r,c) = a(r,0) * b(0,c) + 
               a(r,1) * b(1,c) +
               a(r,2) * b(2,c) +
               a(r,3) * b(3,c);
    }
  }
  return p;
}

Mat4 operator/(const Mat4 &a, float d) {
  return Mat4(
    a(0,0) / d, a(0,1) / d, a(0,2) / d, a(0,3) / d,
    a(1,0) / d, a(1,1) / d, a(1,2) / d, a(1,3) / d,
    a(2,0) / d, a(2,1) / d, a(2,2) / d, a(2,3) / d,
    a(3,0) / d, a(3,1) / d, a(3,2) / d, a(3,3) / d
  );
}

bool operator==(const Mat4 &a, const Mat4 &b) {
  for(int r = 0; r < 4; r++) {
    for(int c = 0; c < 4; c++) {
      if(a(r,c) != b(r,c)) {
        return false;
      }
    }
  }
  return true;
}

bool operator!=(const Mat4 &a, const Mat4 &b) {
  return !(a == b);
}

std::ostream& operator<<(std::ostream& out, const Mat4& m) {
  out << "[ " <<m(0,0)<<' '<<m(0,1)<<' '<<m(0,2)<<' '<<m(0,3) << " ]\n"
      << "[ " <<m(1,0)<<' '<<m(1,1)<<' '<<m(1,2)<<' '<<m(1,3) << " ]\n"
      << "[ " <<m(2,0)<<' '<<m(2,1)<<' '<<m(2,2)<<' '<<m(2,3) << " ]\n"
      << "[ " <<m(3,0)<<' '<<m(3,1)<<' '<<m(3,2)<<' '<<m(3,3) << " ]\n";
  return out;
}

/*****************************************************************************
 * Vec3                                                                      *
 *****************************************************************************/

const Vec3 Vec3::ZERO(0, 0, 0);
const Vec3 Vec3::UNIT_X(1, 0, 0);
const Vec3 Vec3::UNIT_Y(0, 1, 0);
const Vec3 Vec3::UNIT_Z(0, 0, 1);

Vec3::Vec3() {}
Vec3::Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
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

Vec4::Vec4() {}
Vec4::Vec4(float _x, float _y, float _z, float _w)
    : x(_x), y(_y), z(_z), w(_w) {}
Vec4::Vec4(const Vec4 &a) : x(a.x), y(a.y), z(a.z), w(a.w) {}
Vec4::Vec4(const Vec3 &a, float _w) : x(a.x), y(a.y), z(a.z), w(_w) {}

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
    a.r*b.r - a.x*b.x - a.y*b.y - a.z-b.z,
    a.r*b.x + a.x*b.r + a.y*b.z - a.z*b.y,
    a.r*b.y - a.x*b.z + a.y*b.r + a.z*b.x,
    a.r*b.z + a.x*b.y - a.y*b.x + a.z*b.r
  );
}

std::ostream& operator<<(std::ostream& out, const Quat& q) {
  out << q.r << '+' << q.x << "i+" << q.y << "j+" << q.z << 'k';
  return out;
}

/*****************************************************************************
 * Misc                                                                      *
 *****************************************************************************/

Ray::Ray(const Vec3 &_origin, const Vec3 &_direction) :
  origin(_origin), direction(_direction) {}

Vec4 operator*(const Mat4 &m, const Vec4 &v) {
  return Vec4(
    m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z + m(0,3) * v.w,
    m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z + m(1,3) * v.w,
    m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z + m(2,3) * v.w,
    m(3,0) * v.x + m(3,1) * v.y + m(3,2) * v.z + m(3,3) * v.w
  );
}

Ray operator*(const Mat4 &m, const Ray &r) {
  Vec4 o = m * Vec4(r.origin, 1);
  Vec4 d = m * Vec4(r.direction, 0);
  return Ray(o.unHomogenize(), d.dropW());
}

void assertMath() {
  Mat4 a(
     1,  2,  3,  4,
     5,  6,  7,  8,
     9, 10, 11, 12, 
    13, 14, 15, 16
  );

  Mat4 aCopy(a);
  assert(a == aCopy);

  Mat4 aAssign(Mat4::ZERO);
  aAssign = a;
  assert(a == aAssign);

  assert(a - a == Mat4::ZERO);
  assert(a + Mat4::ZERO == a);
  assert(a - Mat4::ZERO == a);
  assert(a * Mat4::IDENTITY == a);
  assert(Mat4::IDENTITY * a == a);
  assert(a * Mat4::ZERO == Mat4::ZERO);

  assert(a.transpose() == Mat4(
    1, 5,  9, 13,
    2, 6, 10, 14,
    3, 7, 11, 15,
    4, 8, 12, 16
  ));

  Mat4 a2;
  for(int r = 0; r < 4; r++) {
    for(int c = 0; c < 4; c++) {
      a2(r,c) = a(r,c) * 2;
    }
  }
  assert(a*2 == a2);
  for(int r = 0; r < 4; r++) {
    for(int c = 0; c < 4; c++) {
      a2(r,c) = a(r,c) / 2;
    }
  }
  assert(a/2 == a2);

  Mat4 b(
     17, -18,  19, -20,
    -21,  22, -23,  24,
     25, -26,  27, -28,
    -29,  30, -31,  32
  );

  assert(a * b == Mat4(
     -66,  68,  -70,  72,
     -98, 100, -102, 104,
    -130, 132, -134, 136,
    -162, 164, -166, 168
  ));

  assert(a != b);

  Vec3 c(1, 4, 8);

  Vec3 cCopy(c);
  assert(c == cCopy);

  Vec3 cAssign(0, 0, 0);
  assert(c == (cAssign = c));
  assert(c == cAssign);

  assert(c.len() == 9);
  assert(c.len2() == 81);
  assert(c.unit() == Vec3(1.0f/9.0f, 4.0f/9.0f, 8.0f/9.0f));

  Vec3 d(-1, 2, -3);

  assert(dot(c, d) == -17);
  assert(cross(c, d) == Vec3(-28, -5, 6));
  assert(proj(Vec3(1,1,1), Vec3(2,0,0)) == Vec3(1,0,0));
  assert(angleBetween(Vec3(1,1,1), Vec3(2,0,0)) == (float) atan(sqrt(2.0f)));
  assert(-c == Vec3(-1, -4, -8));
  assert(c + d == Vec3(0, 6, 5));
  assert(c - d == Vec3(2, 2, 11));
  assert(c * 2 == Vec3(2, 8, 16));
  assert(2 * c == Vec3(2, 8, 16));
  assert(c / 2 == Vec3(0.5, 2, 4));
  assert(c != d);

  assert(Vec3::ZERO + Vec3::UNIT_X + Vec3::UNIT_Y + Vec3::UNIT_Z ==
    Vec3(1, 1, 1));

  Vec4 e(1, 2, 3, 4);

  Vec4 eCopy(e);
  assert(e == eCopy);

  Vec4 eAssign(0, 0, 0, 0);
  eAssign = e;
  assert(e == eAssign);

  Vec4 f(0, -1, 1, -2);

  assert(-e == Vec4(-1, -2, -3, -4));
  assert(e + f == Vec4(1, 1, 4, 2));
  assert(e - f == Vec4(1, 3, 2, 6));
  assert(e != f);

  assert(a * e == Vec4(30, 70, 110, 150));

  assert(Vec4::ZERO + Vec4::UNIT_X + Vec4::UNIT_Y + Vec4::UNIT_Z +
    Vec4::UNIT_W == Vec4(1, 1, 1, 1));

  Quat q1 = Quat::rotation(Vec3(1,1,1), PI/2);
  Quat q2 = Quat::rotation(-Vec3(2,2,2), PI/2);
  Quat q3 = q1 * q2;
  Mat4 q3m = Mat4::rotation(q3);
  assert(q3m == Mat4::IDENTITY);
}
