#pragma once

#include <vm/rt/primitive.hpp>

#include <fmt/core.h>

#include <unordered_map>
#include <array>

namespace vm::rt {

namespace detail {

PrimitiveValue print(size_t count, PrimitiveValue* data);
PrimitiveValue assert(size_t count, PrimitiveValue* data);
PrimitiveValue isNull(size_t count, PrimitiveValue* data);

using IntrinsicType = PrimitiveValue (*)(size_t, PrimitiveValue*);

}  // namespace detail

enum class Intrinsic : uint8_t {
  PRINT,
  ASSERT,
  IS_NULL,
};

static std::unordered_map<std::string, Intrinsic> intrinsics_table{
    {"print", Intrinsic::PRINT},
    {"assert", Intrinsic::ASSERT},
    {"isNull", Intrinsic::IS_NULL},
};

inline detail::IntrinsicType intrinsics_impl[8]{
    detail::print,
    detail::assert,
    detail::isNull,
};

}  // namespace vm::rt
