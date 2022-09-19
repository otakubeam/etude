#pragma once

#include <vm/memory/mem_access.hpp>

#include <vm/stack_v2.hpp>
#include <vm/chunk.hpp>

#include <fmt/color.h>

namespace vm::memory {

class VmMemory {
 public:
  VmMemory() {
  }

  auto AccessMemory(MemAccess descriptor) -> char* {
    // fmt::print(fg(fmt::color::red), "█");

    switch (descriptor.type) {
      case MemAccess::Type::HEAP: {
      }

      case MemAccess::Type::STACK: {
      }

      case MemAccess::Type::ABSOLUTE: {
      }

      case MemAccess::Type::INSTRUCTIONS: {
      }
    }

    std::abort();
  }

  // I want to see it an undicriminated array of bytes
  // But I also want it to have some structure

 private:
  char* memory_ = new char[10000];
  VmStack stack_{memory_ + 1000};
};

}  // namespace vm::memory
