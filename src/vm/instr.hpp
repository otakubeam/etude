#pragma once

#include <fmt/core.h>

#include <cstdlib>

namespace vm {

//////////////////////////////////////////////////////////////////////

enum class InstrType : u_int8_t {
  PUSH_STACK,     // push $1 (index into constant array)
  POP_STACK,      // pop
  RET_FN,         // ret
  CALL_FN,        // call $1 $2 $3 ($1 = chunk, $23 = offset)
  INDIRECT_CALL,  // call (gets the chunk num. from stack)
  JUMP,           // jump $2 $3 ($23 = absolute offset within a chunk)
  JUMP_IF_FALSE,  // jump_if_false $2 $3 ($23 = absolute offset within a chunk)
  ADD,            // add (pops 2 and pushes)
  GET_ARG,    // get_arg $1 (get value at offset -3 - $1 from the current fp)
  GET_LOCAL,  // get_local $1 (get value at offset $1 from the current fp)
  FIN_CALL,   // fin_call $1 (clean up $1 args from the stack and push eax)
  CMP_EQ,     // cmp_eq (pops 2 and pushes)
};

struct Instr {
  InstrType type;

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

inline std::string PrintInstrType(InstrType type) {
  switch (type) {
    case InstrType::CALL_FN:
      return "call_fn";

    case InstrType::RET_FN:
      return "ret_fn";

    case InstrType::GET_ARG:
      return "get_arg";

    case InstrType::GET_LOCAL:
      return "get_local";

    case InstrType::JUMP:
      return "jump";

    case InstrType::JUMP_IF_FALSE:
      return "jump_if_false";

    case InstrType::INDIRECT_CALL:
      return "indirect_call";

    case InstrType::PUSH_STACK:
      return "push_stack";

    case InstrType::ADD:
      return "add";

    case InstrType::FIN_CALL:
      return "fin_call";

    default:
      return std::to_string(uint8_t(type));
  }
}

inline std::string PrintInstr(const Instr& inst) {
  return fmt::format("{}, {} {} {}", PrintInstrType(inst.type), inst.arg1,
                     inst.arg2, inst.arg3);
}

//////////////////////////////////////////////////////////////////////

}  // namespace vm
