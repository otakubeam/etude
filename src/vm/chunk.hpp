#pragma once

#include <vm/rt/primitive.hpp>

#include <vm/instr.hpp>

#include <cstdlib>
#include <vector>

namespace vm {

//////////////////////////////////////////////////////////////////////

struct ExecutableChunk {
  std::vector<Instr> instructions;
  std::vector<rt::PrimitiveValue> attached_vals;
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
