#include <rt/primitive_object.hpp>

// Finally,
#include <catch2/catch.hpp>

using namespace rt;

//////////////////////////////////////////////////////////////////////

TEST_CASE("Rt: Just works", "[rt]") {
  CHECK(PrimitiveObject{1} != PrimitiveObject{'a'});
  CHECK(PrimitiveObject{true} != PrimitiveObject{});

  CHECK(PrimitiveObject{1} == PrimitiveObject{1});
  CHECK(PrimitiveObject{1} != PrimitiveObject{2});
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Throws", "[rt]") {
  CHECK_THROWS_AS(std::get<int>(PrimitiveObject{}),  //
                  std::bad_variant_access);
  CHECK_THROWS_AS(std::get<std::string>(PrimitiveObject{1}),  //
                  std::bad_variant_access);
  CHECK_THROWS_AS(std::get<int>(PrimitiveObject{true}),  //
                  std::bad_variant_access);
  CHECK_THROWS_AS(std::get<std::nullptr_t>(PrimitiveObject{0}),  //
                  std::bad_variant_access);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Binary", "[rt]") {
  CHECK(Plus(PrimitiveObject{1},     //
             PrimitiveObject{2}) ==  //
        //----------------------
        PrimitiveObject{3});

  CHECK(Minus(PrimitiveObject{1},     //
              PrimitiveObject{2}) ==  //
        //----------------------
        PrimitiveObject{-1});
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Unary", "[rt]") {
  CHECK(Negate(PrimitiveObject{1}) == PrimitiveObject{-1});
  CHECK(Bang(PrimitiveObject{true}) == PrimitiveObject{false});
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Prints", "[rt]") {
  CHECK(Format(PrimitiveObject{1}) == "1");
  CHECK(Format(PrimitiveObject{}) == "0x0");
  CHECK(Format(PrimitiveObject{"a"}) == "a");
  CHECK(Format(PrimitiveObject{true}) == "true");
}

//////////////////////////////////////////////////////////////////////
