#include "image.h"

#include "catch.h"

ANON_TEST_CASE() {
  Image a(10, 10, Pixel::V8_T);
  *static_cast<char*>(a.getPixelPtr(5, 5)) = 'A';

  SECTION("move construction") {
    Image b(std::move(a));

    REQUIRE(b.width() == 10);
    REQUIRE(b.height() == 10);
    REQUIRE(b.type() == Pixel::V8_T);
    REQUIRE(b.data() != nullptr);
    REQUIRE(*static_cast<char*>(b.getPixelPtr(5, 5)) == 'A');
  }

  SECTION("move assignment") {
    Image b = std::move(a);

    REQUIRE(b.width() == 10);
    REQUIRE(b.height() == 10);
    REQUIRE(b.type() == Pixel::V8_T);
    REQUIRE(b.data() != nullptr);
    REQUIRE(*static_cast<char*>(b.getPixelPtr(5, 5)) == 'A');
  }

  REQUIRE(a.width() == 0);
  REQUIRE(a.height() == 0);
  REQUIRE(a.type() == Pixel::NONE_T);
  REQUIRE(a.data() == nullptr);
}
