#pragma once

#include <cstdlib>

namespace vm {

//////////////////////////////////////////////////////////////////////

enum class InstrType : u_int8_t {
  PUSH_STACK,  // push $idx
  POP_STACK,   // pop
};

struct Instr {
  InstrType type;

  // Look at the specification in the interpretation
  u_int8_t arg1 = 0;
  u_int8_t arg2 = 0;
  u_int8_t arg3 = 0;
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
