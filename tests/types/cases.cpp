#include <types/builtins.hpp>
#include <types/fn_type.hpp>
#include <types/type.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Just works", "[types]") {
  types::Type* a = &types::builtin_int;
  types::Type* b = &types::builtin_bool;

  CHECK_FALSE(a->IsEqual(b));
  CHECK(a->IsEqual((types::Type*)&types::builtin_int));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Resolution", "[types]") {
  types::Type* fst = new types::FnType{};
  types::Type* fst_beta = new types::FnType{};
  types::Type* snd = new types::BuiltinType{};

  CHECK_FALSE(fst->IsEqual(snd));
  CHECK(fst->IsEqual(fst_beta));
}

//////////////////////////////////////////////////////////////////////
