#include "matrix_vector_product.h"

#include "catch.h"

TEST_CASE("matrix * vector") {
  Matrix<int,3,3> m3x3 {{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
  }};
  Vector3<int> v3 {1,2,3};
  REQUIRE(m3x3 * v3 == Vector3<int>{14,32,50});

  Matrix<int,4,4> m4x4 {{
    { 1,  2,  3,  4},
    { 5,  6,  7,  8},
    { 9, 10, 11, 12},
    {13, 14, 15, 16}
  }};
  Vector4<int> v4 {1,2,3,4};
  REQUIRE(m4x4 * v4 == Vector4<int>{30,70,110,150});
}
