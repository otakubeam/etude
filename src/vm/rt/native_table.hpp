#pragma once

#include <vm/rt/primitive.hpp>

#include <fmt/core.h>

#include <array>

namespace vm::rt {

namespace detail {

// inline PrimitiveValue print(size_t count, PrimitiveValue* data) {
//   // TODO: this will change when strings are implemented
//   for (size_t i = 0; i < count; i++) {
//     fmt::print("{} ", data[(int)-i].as_int);
//   }
//
//   fmt::print("\n\n");
//
//   return PrimitiveValue{};
// }
//
// inline PrimitiveValue isnull(size_t count, PrimitiveValue* data) {
//   FMT_ASSERT(count == 1, "Wrong call for isNull");
//   FMT_ASSERT(data[0].tag == ValueTag::Int, "not a pointer for isNull");
//
//   return PrimitiveValue{
//       .tag = ValueTag::Bool,
//       .as_bool = (data[0].as_int == 1234),
//   };
// }
//
// inline PrimitiveValue sameplace(size_t count, PrimitiveValue* data) {
//   FMT_ASSERT(count == 2, "Wrong call for samePlace");
//   FMT_ASSERT(data[0].tag == ValueTag::Int, "not a pointer for isNull");
//
//   return PrimitiveValue{
//       .tag = ValueTag::Bool,
//       .as_bool = (data[0].as_int == data[-1].as_int),
//   };
// }
//
// inline PrimitiveValue assert(size_t count, PrimitiveValue* data) {
//   FMT_ASSERT(count == 1, "Wrong call for assert");
//   FMT_ASSERT(data[0].tag == ValueTag::Bool, "Non-bool condition");
//
//   if (data[0].as_bool != true) {
//     // die
//     throw "Assertion failed!";
//   }
//
//   return PrimitiveValue{.tag = ValueTag::Bool, .as_bool = true};
// }

}  // namespace detail

// class NativeTable {
//  public:
//   using NativeFuntion = PrimitiveValue (*)(size_t count, PrimitiveValue*);
//   using FunctionStorage = std::array<NativeFuntion, 64>;
//
//   NativeTable(size_t size, FunctionStorage storage)
//       : size_{size}, functions_{std::move(storage)} {
//   }
//
//   static NativeTable SaneDefaults() {
//     return NativeTable{3,
//                        {
//                            // detail::print,
//                            // detail::assert,
//                            // detail::isnull,
//                            // detail::sameplace,
//                        }};
//   }
//
//   void Add(NativeFuntion fn) {
//     functions_[size_] = fn;
//   }
//
//   NativeFuntion Get(size_t offset) {
//     FMT_ASSERT(offset < size_, "Bad offset");
//     return functions_[offset];
//   }
//
//  private:
//   size_t size_ = 0;
//   std::array<NativeFuntion, 64> functions_;
// };

}  // namespace vm::rt
