
#include <vm/rt/primitive.hpp>

#include <fmt/core.h>

#include <unordered_map>

namespace vm::rt {

namespace detail {

PrimitiveValue print(size_t count, PrimitiveValue* data) {
  for (size_t i = 0; i < count; i++) {
    fmt::print("{} ", FormatPrimitiveValue(data[(int)-i]));
  }

  fmt::print("\n\n");

  return PrimitiveValue{};
}

PrimitiveValue assert(size_t count, PrimitiveValue* data) {
  FMT_ASSERT(count == 1, "Wrong call for assert");
  FMT_ASSERT(data[0].tag == ValueTag::Bool, "Non-bool condition");

  if (data[0].as_bool != true) {
    // die
    throw "Assertion failed!";
  }

  return PrimitiveValue{};
}

PrimitiveValue isNull(size_t count, PrimitiveValue* stack_top) {
  FMT_ASSERT(count == 1, "Wrong call for isNull");

  stack_top[0] = {
      .tag = rt::ValueTag::Bool,
      .as_bool = stack_top[0].tag == rt::ValueTag::Unit,
  };

  return PrimitiveValue{};
}

}  // namespace detail

}  // namespace vm::rt
