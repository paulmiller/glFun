#include "util.h"

#include "catch.h"

using namespace Catch::literals;

TEST_CASE("LinearMap") {
  REQUIRE(LinearMap_d( 0  ,   0,   1,   0,  10) ==  0_a);
  REQUIRE(LinearMap_d( 1  ,   0,   1,   0,  10) == 10_a);
  REQUIRE(LinearMap_d( 0.5,   0,   1,   0,  10) ==  5_a);
  REQUIRE(LinearMap_d( 0.5,   0,   1,   0, -10) == -5_a);
  REQUIRE(LinearMap_d(-1  , -10,  10,   1, - 1) ==  0.1_a);
}

TEST_CASE("IsPowerOf2") {
  REQUIRE(!IsPowerOf2(-2));
  REQUIRE(!IsPowerOf2(-1));
  REQUIRE(!IsPowerOf2( 0));
  REQUIRE( IsPowerOf2( 1));
  REQUIRE( IsPowerOf2( 2));
  REQUIRE(!IsPowerOf2( 3));
  REQUIRE( IsPowerOf2( 4));

  REQUIRE(!IsPowerOf2(0u));
  REQUIRE( IsPowerOf2(1u));
  REQUIRE( IsPowerOf2(2u));
  REQUIRE(!IsPowerOf2(3u));
  REQUIRE( IsPowerOf2(4u));

  REQUIRE(IsPowerOf2(65536));
  REQUIRE(IsPowerOf2(65536u));
}
