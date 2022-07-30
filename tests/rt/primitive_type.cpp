#include <rt/primitive_type.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Rt: Just works", "[rt]") {
  CHECK(PrimitiveType{1} != PrimitiveType{'a'});
  CHECK(PrimitiveType{true} != PrimitiveType{});

  CHECK(PrimitiveType{1} == PrimitiveType{1});
  CHECK(PrimitiveType{1} != PrimitiveType{2});
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Throws", "[rt]") {
  CHECK_THROWS_AS(std::get<int>(PrimitiveType{}),  //
                  std::bad_variant_access);
  CHECK_THROWS_AS(std::get<std::string>(PrimitiveType{1}),  //
                  std::bad_variant_access);
  CHECK_THROWS_AS(std::get<int>(PrimitiveType{true}),  //
                  std::bad_variant_access);
  CHECK_THROWS_AS(std::get<std::nullptr_t>(PrimitiveType{0}),  //
                  std::bad_variant_access);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Binary", "[rt]") {
  CHECK(Plus(PrimitiveType{1},     //
             PrimitiveType{2}) ==  //
        //----------------------
        PrimitiveType{3});

  CHECK(Minus(PrimitiveType{1},     //
              PrimitiveType{2}) ==  //
        //----------------------
        PrimitiveType{-1});
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Unary", "[rt]") {
  CHECK(Negate(PrimitiveType{1}) == PrimitiveType{-1});
  CHECK(Bang(PrimitiveType{true}) == PrimitiveType{false});
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Prints", "[rt]") {
  CHECK(Format(PrimitiveType{1}) == "1");
  CHECK(Format(PrimitiveType{}) == "0x0");
  CHECK(Format(PrimitiveType{"a"}) == "a");
  CHECK(Format(PrimitiveType{true}) == "true");
}

//////////////////////////////////////////////////////////////////////
