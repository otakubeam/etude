#pragma once

#include <vm/chunk.hpp>
#include <vm/stack.hpp>

namespace vm::memory {

struct MemAccess {
  enum class Type {
    HEAP,
    STACK,
    INSTRUCTIONS,
  };

  Type type;
  size_t offset;
};

}  // namespace vm::memory
