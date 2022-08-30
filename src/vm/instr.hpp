#pragma once

#include <cstdlib>

namespace vm {

//////////////////////////////////////////////////////////////////////

enum class InstrType : u_int8_t {
  PUSH_STACK,     // push $1 (index into constant array)
  POP_STACK,      // pop
  RET_FN,         // ret
  CALL_FN,        // call $1 $2 $3 ($1 = chunk, $23 = offset)
  JUMP_IF_FALSE,  // jump_if_false $1 $2 ($12 = absolute offset)
};

struct Instr {
  InstrType type;

  // Look at the specification in the interpretation
  u_int8_t arg1 = 0;
  u_int8_t arg2 = 0;
  u_int8_t arg3 = 0;
};

inline u_int8_t ReadByte(const Instr& inst) {
  return inst.arg1;
}

inline u_int16_t ReadWord(const Instr& inst) {
  return ((inst.arg1 << 8) + inst.arg2) & 0xFF;
}

//////////////////////////////////////////////////////////////////////

}  // namespace vm
