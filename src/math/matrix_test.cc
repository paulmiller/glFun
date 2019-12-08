#include "matrix.h"

#include "catch.h"

namespace {
  const Matrix<int,1,1> a1x1 {{{1}}};

  const Matrix<int,2,2> a2x2 {{
    {1, 2},
    {3, 4},
  }};

  const Matrix<int,3,3> a3x3 {{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
  }};

  const Matrix<int,3,3> b3x3 {{
    {2,  3,  4},
    {5,  6,  7},
    {8,  9, 10}
  }};

  const Matrix<int,3,4> a3x4 {{
    {1,  2,  3,  4},
    {5,  6,  7,  8},
    {9, 10, 11, 12}
  }};

  const Matrix<int,4,3> a4x3 {{
    { 1,  2,  3},
    { 4,  5,  6},
    { 7,  8,  9},
    {10, 11, 12}
  }};
}

TEST_CASE("global Matrix values") {
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      REQUIRE(Zero_Matrix3x3f(r,c) == 0.0f);
      REQUIRE(Zero_Matrix3x3d(r,c) == 0.0);

      bool diagonal = (r == c);
      REQUIRE(Identity_Matrix3x3f(r,c) == (diagonal ? 1.0f : 0.0f));
      REQUIRE(Identity_Matrix3x3d(r,c) == (diagonal ? 1.0 : 0.0));
    }
  }

  for(int r = 0; r < 4; r++) {
    for(int c = 0; c < 4; c++) {
      REQUIRE(Zero_Matrix4x4f(r,c) == 0.0f);
      REQUIRE(Zero_Matrix4x4d(r,c) == 0.0);

      bool diagonal = (r == c);
      REQUIRE(Identity_Matrix4x4f(r,c) == (diagonal ? 1.0f : 0.0f));
      REQUIRE(Identity_Matrix4x4d(r,c) == (diagonal ? 1.0 : 0.0));
    }
  }
}

TEST_CASE("Matrix initialization") {
  Matrix<int,3,3> m{{
    { 0,  1,  2},
    {10, 11, 12},
    {20, 21, 22}
  }};

  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      REQUIRE(m(r,c) == r * 10 + c);
    }
  }
}

TEST_CASE("Matrix transpose") {
  auto t1x1 = a1x1.transpose();
  REQUIRE(t1x1(0,0) == 1);

  auto t2x2 = a2x2.transpose();
  for(int r = 0; r < 2; r++) {
    for(int c = 0; c < 2; c++) {
      REQUIRE(a2x2(r,c) == t2x2(c,r));
    }
  }

  auto t4x3 = a3x4.transpose();
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 4; c++) {
      REQUIRE(a3x4(r,c) == t4x3(c,r));
    }
  }
}

TEST_CASE("Matrix determinant") {
  REQUIRE(a1x1.determinant() == 1);
  REQUIRE(a2x2.determinant() == -2);
  REQUIRE(a3x3.determinant() == 0);
}

TEST_CASE("Matrix isfinite") {
  Matrix<float,2,2> a = {{
    {1, 2},
    {3, 4}
  }};
  REQUIRE(a.isfinite());

  Matrix<float,2,2> b = {{
    {1, INFINITY},
    {3, 4}
  }};
  REQUIRE(!b.isfinite());

  Matrix<float,2,2> c = {{
    {1, 2},
    {3, -INFINITY}
  }};
  REQUIRE(!c.isfinite());

  Matrix<float,2,2> d = {{
    {1, 2},
    {NAN, 4}
  }};
  REQUIRE(!d.isfinite());
}

TEST_CASE("Matrix scalar multiplication") {
  Matrix<int,3,3> product{{
    { 2,  4,  6},
    { 8, 10, 12},
    {14, 16, 18}
  }};
  REQUIRE(a3x3 * 2 == product);
  REQUIRE(2 * a3x3 == product);

  auto c3x3 = a3x3;
  c3x3 *= 2;
  REQUIRE(c3x3 == product);
}

TEST_CASE("Matrix scalar division") {
  Matrix3x3d a3x3d{{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
  }};
  Matrix3x3d b3x3d{{
    {0.5, 1  , 1.5},
    {2  , 2.5, 3  },
    {3.5, 4  , 4.5}
  }};
  REQUIRE(a3x3d / 2 == b3x3d);

  auto c3x3d = a3x3d;
  c3x3d /= 2;
  REQUIRE(c3x3d == b3x3d);
}

TEST_CASE("Matrix equality") {
  REQUIRE(a1x1 == a1x1);
  auto b2x2 = a2x2;
  REQUIRE(a2x2 == b2x2);
  b2x2(1,1) = 0;
  REQUIRE(a2x2 != b2x2);
  REQUIRE(a3x3 != b3x3);
  REQUIRE(a3x4 == a3x4);
}

TEST_CASE("Matrix negation") {
  auto b3x4 = a3x4;
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 4; c++) {
      b3x4(r,c) = -b3x4(r,c);
    }
  }
  REQUIRE(a3x4 == -b3x4);
}

TEST_CASE("Matrix addition") {
  Matrix<int,3,3> sum{{
    { 3,  5,  7},
    { 9, 11, 13},
    {15, 17, 19}
  }};
  REQUIRE(a3x3 + b3x3 == sum);

  auto c3x3 = a3x3;
  c3x3 += b3x3;
  REQUIRE(c3x3 == sum);
}

TEST_CASE("Matrix subtraction") {
  Matrix<int,3,3> difference{{
    {-1, -1, -1},
    {-1, -1, -1},
    {-1, -1, -1}
  }};
  REQUIRE(a3x3 - b3x3 == difference);

  auto c3x3 = a3x3;
  c3x3 -= b3x3;
  REQUIRE(c3x3 == difference);
}

TEST_CASE("Matrix multiplication") {
  // TODO 2x1 x 1x1

  Matrix<int,2,2> product2x2{{
    { 7, 10},
    {15, 22}
  }};
  REQUIRE(a2x2 * a2x2 == product2x2);

  auto b2x2 = a2x2;
  b2x2 *= a2x2;
  REQUIRE(b2x2 == product2x2);

  Matrix<int,3,3> product3x3{{
    { 70,  80,  90},
    {158, 184, 210},
    {246, 288, 330}
  }};
  REQUIRE(a3x4 * a4x3 == product3x3);
}
