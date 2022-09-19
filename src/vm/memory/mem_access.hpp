#pragma once

#include <cstddef>

namespace vm::memory {

struct MemAccess {
  enum class Type {
    HEAP,
    STACK,
    INSTRUCTIONS,
    ABSOLUTE,
  };

  Type type;

  bool store = false;

  size_t offset = 0;
};

}  // namespace vm::memory
