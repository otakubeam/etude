#include <types/repr/builtins.hpp>
#include <types/repr/fn_type.hpp>
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

TEST_CASE("Complex fn comparison", "[types]") {
  types::Type* a = &types::builtin_int;
  types::Type* b = &types::builtin_bool;

  types::Type* fst = new types::FnType{
      {a, a, b, a}, new types::FnType{{a, b}, &types::builtin_unit}};
  types::Type* snd = new types::FnType{
      {a, a, b, a}, new types::FnType{{a, b}, &types::builtin_unit}};

  types::Type* not_fst_alpha = new types::FnType{
      {a, a, b, a}, new types::FnType{{b}, &types::builtin_unit}};
  types::Type* not_fst_beta = new types::FnType{
      {a, b, b, a}, new types::FnType{{b}, &types::builtin_unit}};

  CHECK(fst->IsEqual(snd));
  CHECK_FALSE(fst->IsEqual(not_fst_alpha));
  CHECK_FALSE(fst->IsEqual(not_fst_beta));
}

//////////////////////////////////////////////////////////////////////
