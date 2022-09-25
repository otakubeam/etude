#pragma once

#include <vm/rt/primitive.hpp>

#include <cstddef>

namespace vm::memory {

// Token to access memory

struct MemAccess {
  rt::Reference mem_ref;
  rt::ValueTag type;
  bool store = false;
};

}  // namespace vm::memory
