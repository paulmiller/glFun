#include "math/matrix_factories.h"

#include "math/util.h"
#include "math/vector.h"
#include "math/matrix_test_util.h"

#include "catch.h"

TEST_CASE("MatrixFromColumnVectors") {
  auto actual3x3 = MatrixFromColumnVectors(
    Vector3<int>{1, 2, 3},
    Vector3<int>{4, 5, 6},
    Vector3<int>{7, 8, 9}
  );
  Matrix<int,3,3> expected3x3 {{
    {1, 4, 7},
    {2, 5, 8},
    {3, 6, 9}
  }};
  REQUIRE(actual3x3 == expected3x3);

  auto actual4x4 = MatrixFromColumnVectors(
    Vector4<int>{ 1,  2,  3,  4},
    Vector4<int>{ 5,  6,  7,  8},
    Vector4<int>{ 9, 10, 11, 12},
    Vector4<int>{13, 14, 15, 16}
  );
  Matrix<int,4,4> expected4x4 {{
    {1,  5,  9, 13},
    {2,  6, 10, 14},
    {3,  7, 11, 15},
    {4,  8, 12, 16}
  }};
  REQUIRE(actual4x4 == expected4x4);
}

TEST_CASE("RotationMatrix4x4") {
  using namespace Catch::literals;

  Matrix<double,4,4> actual;
  ApproxMatrix<4,4> expected;
  expected.SetMargin(1e-10);

  // quarter turns around the X axis
  expected.matrix = Matrix4x4d {{
    { 1, 0, 0, 0},
    { 0, 0,-1, 0},
    { 0, 1, 0, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(UnitX_Vector3d, Tau_d/4);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(-UnitX_Vector3d, -Tau_d/4);
  REQUIRE(actual == expected);
  expected.matrix = Matrix4x4d {{
    { 1, 0, 0, 0},
    { 0, 0, 1, 0},
    { 0,-1, 0, 0},
    { 0, 0, 0, 1}
  }};
  actual = RotationMatrix4x4(UnitX_Vector3d, -Tau_d/4);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(-UnitX_Vector3d, Tau_d/4);
  REQUIRE(actual == expected);

  // quarter turns around the Y axis
  expected.matrix = Matrix4x4d {{
    { 0, 0, 1, 0},
    { 0, 1, 0, 0},
    {-1, 0, 0, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(UnitY_Vector3d, Tau_d/4);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(-UnitY_Vector3d, -Tau_d/4);
  REQUIRE(actual == expected);
  expected.matrix = Matrix4x4d {{
    { 0, 0,-1, 0},
    { 0, 1, 0, 0},
    { 1, 0, 0, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(UnitY_Vector3d, -Tau_d/4);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(-UnitY_Vector3d, Tau_d/4);
  REQUIRE(actual == expected);

  // quarter turns around the around Z axis
  expected.matrix = Matrix4x4d {{
    { 0,-1, 0, 0},
    { 1, 0, 0, 0},
    { 0, 0, 1, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(UnitZ_Vector3d, Tau_d/4);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(-UnitZ_Vector3d, -Tau_d/4);
  REQUIRE(actual == expected);
  expected.matrix = Matrix4x4d {{
    { 0, 1, 0, 0},
    {-1, 0, 0, 0},
    { 0, 0, 1, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(UnitZ_Vector3d, -Tau_d/4);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(-UnitZ_Vector3d, Tau_d/4);
  REQUIRE(actual == expected);

  // 1/3 turn around <1,1,1>
  expected.matrix = Matrix4x4d {{
    { 0, 0, 1, 0},
    { 1, 0, 0, 0},
    { 0, 1, 0, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(Vector3d{1,1,1}.unit(), Tau_d/3);
  REQUIRE(actual == expected);

  // no-op turns around an arbitrary axis
  Vector3d axis = Vector3d{1,2,3}.unit();
  expected.matrix = Matrix4x4d {{
    { 1, 0, 0, 0},
    { 0, 1, 0, 0},
    { 0, 0, 1, 0},
    { 0, 0, 0, 1}}};
  actual = RotationMatrix4x4(axis, 0);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(axis, Tau_d);
  REQUIRE(actual == expected);
  actual = RotationMatrix4x4(axis, -Tau_d);
  REQUIRE(actual == expected);
}

TEST_CASE("ScaleMatrix4x4") {
  auto actual = ScaleMatrix4x4(Vector3<int>{2,3,4});
  Matrix<int,4,4> expected {{
    {2, 0, 0, 0},
    {0, 3, 0, 0},
    {0, 0, 4, 0},
    {0, 0, 0, 1}
  }};
  REQUIRE(actual == expected);
}
