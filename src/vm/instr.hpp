#pragma once

#include <cstdlib>

namespace vm {

//////////////////////////////////////////////////////////////////////

enum class InstrType : u_int8_t {
  PUSH_STACK,     // push $1 (index into constant array)
  POP_STACK,      // pop
  RET_FN,         // ret
  CALL_FN,        // call $1 $2 $3 ($1 = chunk, $23 = offset)
  JUMP,           // jump $2 $3 ($23 = absolute offset within a chunk)
  JUMP_IF_FALSE,  // jump_if_false $2 $3 ($23 = absolute offset within a chunk)
  ADD,            // add (pops 2 and pushes)
  FROM_STACK,     // from_stack $1 (get value at offset $1 from the current fp)
  FIN_CALL,       // fin_call $1 (clean up $1 args from the stack and push eax)
  CMP_EQ,         // cmp_eq (pops 2 and pushes)
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
  return ((inst.arg2 << 8) + inst.arg3);
}

//////////////////////////////////////////////////////////////////////

}  // namespace vm
