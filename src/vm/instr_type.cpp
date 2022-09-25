#include <vm/instr_type.hpp>

#include <string>

namespace vm {

std::string PrintInstrType(InstrType type) {
  switch (type) {
    case InstrType::CALL_FN:
      return "call_fn";

    case InstrType::NATIVE_CALL:
      return "native_call";

    case InstrType::RET_FN:
      return "ret_fn";

    case InstrType::POP_STACK:
      return "pop";

    case InstrType::GET_ARG:
      return "get_arg";

    case InstrType::GET_LOCAL:
      return "get_local";

    case InstrType::LOAD:
      return "load";

    case InstrType::STORE:
      return "store";

    case InstrType::JUMP:
      return "jump";

    case InstrType::JUMP_IF_FALSE:
      return "jump_if_false";

    case InstrType::INDIRECT_CALL:
      return "indirect_call";

    case InstrType::ADD:
      return "add";

    case InstrType::FIN_CALL:
      return "fin_call";

    case InstrType::CMP_EQ:
      return "cmp_eq";

    case InstrType::CMP_LESS:
      return "cmp_less";

    default:
      return std::to_string(uint8_t(type));
  }
}

}  // namespace vm
