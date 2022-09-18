#pragma once

#include <vm/chunk.hpp>
#include <vm/stack.hpp>

namespace vm::memory {

class VmMemory {
 public:
  VmMemory() {
  }

  // I want to see it an undicriminated array of bytes
  // But I also want it to have some structure

 private:
  VmStack stack_;
};

}  // namespace vm::memory
