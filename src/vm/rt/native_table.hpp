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

inline detail::IntrinsicType intrinsics_impl[8]{
    detail::print,
    detail::assert,
    detail::isNull,
};

}  // namespace vm::rt
