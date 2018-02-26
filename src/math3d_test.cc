#include "math3d.h"

#include "util.h"

#include "catch.h"

ANON_TEST_CASE() {
  Mat4 a(
     1,  2,  3,  4,
     5,  6,  7,  8,
     9, 10, 11, 12, 
    13, 14, 15, 16
  );

  Mat4 aCopy(a);
  REQUIRE(a == aCopy);

  Mat4 aAssign(Mat4::ZERO);
  aAssign = a;
  REQUIRE(a == aAssign);

  REQUIRE(a - a == Mat4::ZERO);
  REQUIRE(a + Mat4::ZERO == a);
  REQUIRE(a - Mat4::ZERO == a);
  REQUIRE(a * Mat4::IDENTITY == a);
  REQUIRE(Mat4::IDENTITY * a == a);
  REQUIRE(a * Mat4::ZERO == Mat4::ZERO);

  REQUIRE(a.transpose() == Mat4(
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
  REQUIRE(a*2 == a2);
  for(int r = 0; r < 4; r++) {
    for(int c = 0; c < 4; c++) {
      a2(r,c) = a(r,c) / 2;
    }
  }
  REQUIRE(a/2 == a2);

  Mat4 b(
     17, -18,  19, -20,
    -21,  22, -23,  24,
     25, -26,  27, -28,
    -29,  30, -31,  32
  );

  REQUIRE(a * b == Mat4(
     -66,  68,  -70,  72,
     -98, 100, -102, 104,
    -130, 132, -134, 136,
    -162, 164, -166, 168
  ));

  REQUIRE(a != b);
}

ANON_TEST_CASE() {
  Vec2 a(4, 3);

  Vec2 aCopy(a);
  REQUIRE(a == aCopy);

  Vec2 aAssign(0, 0);
  REQUIRE(a == (aAssign = a));
  REQUIRE(a == aAssign);

  REQUIRE(a.len() == 5);
  REQUIRE(a.len2() == 25);
  REQUIRE(a.unit() == Vec2(4.0f/5.0f, 3.0f/5.0f));

  Vec2 b(-1, 2);

  REQUIRE(dot(a, b) == 2);
  REQUIRE(proj(Vec2(1,1), Vec2(2,0)) == Vec2(1,0));
  REQUIRE(angleBetween(Vec2(1,1), Vec2(2,0)) == PI_f/4);
  REQUIRE(-a == Vec2(-4, -3));
  REQUIRE(a + b == Vec2(3, 5));
  REQUIRE(a - b == Vec2(5, 1));
  REQUIRE(a * 2 == Vec2(8, 6));
  REQUIRE(2 * a == Vec2(8, 6));
  REQUIRE(a / 2 == Vec2(2, 1.5f));
  REQUIRE(a != b);

  REQUIRE(Vec2::ZERO + Vec2::UNIT_X + Vec2::UNIT_Y == Vec2(1, 1));
}

void testVec3() {
  Vec3 a(1, 4, 8);

  Vec3 aCopy(a);
  REQUIRE(a == aCopy);

  Vec3 aAssign(0, 0, 0);
  REQUIRE(a == (aAssign = a));
  REQUIRE(a == aAssign);

  REQUIRE(a.len() == 9);
  REQUIRE(a.len2() == 81);
  REQUIRE(a.unit() == Vec3(1.0f/9.0f, 4.0f/9.0f, 8.0f/9.0f));

  Vec3 b(-1, 2, -3);

  REQUIRE(dot(a, b) == -17);
  REQUIRE(proj(Vec3(1,1,1), Vec3(2,0,0)) == Vec3(1,0,0));
  float angle = atan(sqrt(2.0f));
  REQUIRE(angleBetween(Vec3(1,1,1), Vec3(2,0,0)) == angle);
  REQUIRE(quatBetween(Vec3(2,0,0), Vec3(1,1,1)) ==
    Quat::rotation(Vec3(0,-1,1), angle));
  REQUIRE(-a == Vec3(-1, -4, -8));
  REQUIRE(a + b == Vec3(0, 6, 5));
  REQUIRE(a - b == Vec3(2, 2, 11));
  REQUIRE(a * 2 == Vec3(2, 8, 16));
  REQUIRE(2 * a == Vec3(2, 8, 16));
  REQUIRE(a / 2 == Vec3(0.5, 2, 4));
  REQUIRE(a != b);

  REQUIRE(Vec3::ZERO + Vec3::UNIT_X + Vec3::UNIT_Y + Vec3::UNIT_Z ==
    Vec3(1, 1, 1));
}

ANON_TEST_CASE() {
  Vec4 a(1, 2, 3, 4);

  Vec4 aCopy(a);
  REQUIRE(a == aCopy);

  Vec4 aAssign(0, 0, 0, 0);
  aAssign = a;
  REQUIRE(a == aAssign);

  Vec4 b(0, -1, 1, -2);

  REQUIRE(-a == Vec4(-1, -2, -3, -4));
  REQUIRE(a + b == Vec4(1, 1, 4, 2));
  REQUIRE(a - b == Vec4(1, 3, 2, 6));
  REQUIRE(a != b);

  Mat4 m(
     1,  2,  3,  4,
     5,  6,  7,  8,
     9, 10, 11, 12, 
    13, 14, 15, 16
  );
  REQUIRE(m * a == Vec4(30, 70, 110, 150));

  REQUIRE(Vec4::ZERO + Vec4::UNIT_X + Vec4::UNIT_Y + Vec4::UNIT_Z +
    Vec4::UNIT_W == Vec4(1, 1, 1, 1));
}

ANON_TEST_CASE() {
  Quat q1 = Quat::rotation(Vec3(1,1,1), PI_f/2);
  Quat q2 = Quat::rotation(-Vec3(2,2,2), PI_f/2);
  Quat q3 = q1 * q2;
  Mat4 q3m = Mat4::rotation(q3);
  REQUIRE(q3m == Mat4::IDENTITY);
  REQUIRE(Quat(1,-2,3,-4) * Quat(-5,6,-7,8) == Quat(60,12,-30,24));
}

ANON_TEST_CASE() {
  REQUIRE(   0 == det(Vec3( 1, 4, 7), Vec3( 2, 5, 8), Vec3( 3, 6, 9)) );
  REQUIRE( 140 == det(Vec3( 9, 7,-3), Vec3( 4, 2,-8), Vec3( 4, 5,-4)) );
}
