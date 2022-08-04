#include <rt/base_object.hpp>

// Finally,
#include <catch2/catch.hpp>

using namespace rt;

//////////////////////////////////////////////////////////////////////

TEST_CASE("From Primitive", "[rt]") {
  FromPrim('a');
  FromPrim(123);
  FromPrim(false);
  FromPrim(nullptr);
}

TEST_CASE("Object format", "[rt]") {
  CHECK(Format(FromPrim(123)) == "123");
}

//////////////////////////////////////////////////////////////////////
