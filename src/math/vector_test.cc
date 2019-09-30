#include "math/vector.h"

#include "math/util.h"
#include "math/vector_test_util.h"

#include "catch.h"

TEST_CASE("Vector initialization") {
  Vector3<int> v3 {1,2,3};
  REQUIRE(v3.x == 1);
  REQUIRE(v3.y == 2);
  REQUIRE(v3.z == 3);

  Vector3<int> v3_0 {};
  REQUIRE(v3_0.x == 0);
  REQUIRE(v3_0.y == 0);
  REQUIRE(v3_0.z == 0);

  Vector4<int> v4 {1,2,3,4};
  REQUIRE(v4.x == 1);
  REQUIRE(v4.y == 2);
  REQUIRE(v4.z == 3);
  REQUIRE(v4.w == 4);

  Vector4<int> v4_0 {};
  REQUIRE(v4_0.x == 0);
  REQUIRE(v4_0.y == 0);
  REQUIRE(v4_0.z == 0);
  REQUIRE(v4_0.w == 0);
}

TEST_CASE("Vector3 length") {
  // 2^2 + 3^2 + 6^2 = 7^2
  Vector3d v {2,3,6};
  REQUIRE(v.len() == 7);
  REQUIRE(v.len2() == 7*7);
  REQUIRE(v.unit() == Vector3d {2.0/7.0, 3.0/7.0, 6.0/7.0});
}

TEST_CASE("vector dot product") {
  REQUIRE(dot(Vector3<int>{1,2,3}, Vector3<int>{4,5,6}) == 32);
  REQUIRE(dot(Vector4<int>{1,2,3,4}, Vector4<int>{5,6,7,8}) == 70);
}

TEST_CASE("Vector3 cross product") {
  REQUIRE(cross(Vector3<int>{1,2,3}, Vector3<int>{4,5,6}) ==
    Vector3<int>{-3,6,-3});
}

TEST_CASE("Vector3 projection") {
  Vector3d v {1,1,1};
  REQUIRE(proj(v, Vector3d{10,0,0}) == ApproxVector3(Vector3d{1,0,0}));
  REQUIRE(proj(v, Vector3d{0,0.5,0.5}) == ApproxVector3(Vector3d{0,1,1}));
}

TEST_CASE("Vector3 angles") {
  REQUIRE(angleBetween(UnitX_Vector3d, UnitX_Vector3d) == 0);
  REQUIRE(angleBetween(UnitX_Vector3d, UnitY_Vector3d) == Approx(Tau_d/4));
  REQUIRE(angleBetween(UnitY_Vector3d, UnitZ_Vector3d) == Approx(Tau_d/4));
  REQUIRE(angleBetween(UnitX_Vector3d, Vector3d{0,100,0})
    == Approx(Tau_d/4));
  REQUIRE(angleBetween(Vector3d{Root2_d,1,1}, Vector3d{-Root2_d,1,1}) ==
    Approx(Tau_d/4));
  REQUIRE(angleBetween(Vector3d{1,1,1}, Vector3d{-1,-1,-1}) ==
    Approx(Tau_d/2));
}
