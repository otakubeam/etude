#include <vm/instr_type.hpp>

#include <string>

namespace vm {

std::string FormatInstrType(InstrType type) {
  switch (type) {
    case InstrType::PUSH_TRUE:
      return "push_true";

    case InstrType::PUSH_FALSE:
      return "push_false";

    case InstrType::PUSH_UNIT:
      return "push_unit";

    case InstrType::SUBTRACT:
      return "subtract";

    case InstrType::PUSH_VALUE:
      return "push_value";

    case InstrType::CALL_FN:
      return "call_fn";

    case InstrType::NATIVE_CALL:
      return "native_call";

    case InstrType::RET_FN:
      return "ret_fn";

    case InstrType::POP_STACK:
      return "pop";

    case InstrType::PUSH_FP:
      return "push_fp";

    case InstrType::ADD_ADDR:
      return "add_addr";

    case InstrType::LOAD:
      return "load";

    case InstrType::STORE:
      return "store";

    case InstrType::GET_AT_FP:
      return "get_at_fp";

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

    case InstrType::CMP_GE:
      return "cmp_ge";

    case InstrType::CMP_GREATER:
      return "cmp_greater";

    case InstrType::CMP_LE:
      return "cmp_le";

    case InstrType::ALLOC:
      return "alloc";

    default:
      return std::to_string(uint8_t(type));
  }
}

}  // namespace vm
