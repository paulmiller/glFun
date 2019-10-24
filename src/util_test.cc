#include "util.h"

#include "function_traits.h"

#include "catch.h"

#include <limits>
#include <regex>

TEST_CASE("PrettyPrintNumBytes") {
  using T = function_traits<decltype(PrettyPrintNumBytes)>::argument<0>::type;
  constexpr T K = 1 << 10, M = 1 << 20, G = 1 << 30;

  // show exact numbers up to 1 KiB
  REQUIRE(PrettyPrintNumBytes(0) == "0 B");
  REQUIRE(PrettyPrintNumBytes(1) == "1 B");
  REQUIRE(PrettyPrintNumBytes(2) == "2 B");
  REQUIRE(PrettyPrintNumBytes(K - 1) == "1023 B");
  REQUIRE(PrettyPrintNumBytes(K    ) == "1 KiB");
  REQUIRE(PrettyPrintNumBytes(K + 1) == "1 KiB");

  // round up to 2 KiB starting at 1.5 KiB
  REQUIRE(PrettyPrintNumBytes(K + 511) == "1 KiB");
  REQUIRE(PrettyPrintNumBytes(K + 512) == "2 KiB");
  REQUIRE(PrettyPrintNumBytes(K + 513) == "2 KiB");

  // round up to 3 KiB starting at 2.5 KiB
  REQUIRE(PrettyPrintNumBytes(2*K + 511) == "2 KiB");
  REQUIRE(PrettyPrintNumBytes(2*K + 512) == "3 KiB");
  REQUIRE(PrettyPrintNumBytes(2*K + 513) == "3 KiB");

  // round up to 1 MiB starting at 1023.5 KiB
  REQUIRE(PrettyPrintNumBytes(M - 513) == "1023 KiB");
  REQUIRE(PrettyPrintNumBytes(M - 512) == "1 MiB");
  REQUIRE(PrettyPrintNumBytes(M - 511) == "1 MiB");

  // round up to 2 MiB starting at 1.5 MiB
  REQUIRE(PrettyPrintNumBytes(M + 512*K - 1) == "1 MiB");
  REQUIRE(PrettyPrintNumBytes(M + 512*K    ) == "2 MiB");
  REQUIRE(PrettyPrintNumBytes(M + 512*K + 1) == "2 MiB");

  // round up to 3 MiB starting at 2.5 MiB
  REQUIRE(PrettyPrintNumBytes(2*M + 512*K - 1) == "2 MiB");
  REQUIRE(PrettyPrintNumBytes(2*M + 512*K    ) == "3 MiB");
  REQUIRE(PrettyPrintNumBytes(2*M + 512*K + 1) == "3 MiB");

  // round up to 1 GiB starting at 1023.5 MiB
  REQUIRE(PrettyPrintNumBytes(G - 512*K - 1) == "1023 MiB");
  REQUIRE(PrettyPrintNumBytes(G - 512*K    ) == "1 GiB");
  REQUIRE(PrettyPrintNumBytes(G - 512*K + 1) == "1 GiB");

  constexpr T Max = std::numeric_limits<T>::max();
  REQUIRE(std::regex_match(PrettyPrintNumBytes(Max), std::regex("\\d+ EiB")));
}
