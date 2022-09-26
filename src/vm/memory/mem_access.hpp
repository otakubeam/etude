#pragma once

#include <vm/rt/primitive.hpp>

#include <cstddef>

namespace vm::memory {

// Token to access memory

struct MemAccess {
  rt::PrimitiveValue reference;
  bool store = false;
};

}  // namespace vm::memory
