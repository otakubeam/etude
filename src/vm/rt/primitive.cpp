#include <vm/rt/primitive.hpp>

namespace vm::rt {

std::string FormatValueTag(ValueTag tag) {
  switch (tag) {
    case ValueTag::Int:
      return "Int";
    case ValueTag::Bool:
      return "Bool";
    case ValueTag::Unit:
      return "Unit";
    case ValueTag::Char:
      return "Char";
    case ValueTag::HeapRef:
      return "HeapRef";
    case ValueTag::StackRef:
      return "StackRef";
    case ValueTag::InstrRef:
      return "InstrRef";
    case ValueTag::StaticRef:
      return "StaticRef";
    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

std::string FormatInstrRef(InstrReference instr_ref) {
  return fmt::format("[{} {}]", instr_ref.chunk_no, instr_ref.instr_no);
}

std::string FormatValue(PrimitiveValue value) {
  switch (value.tag) {
    case ValueTag::Unit:
      return fmt::format("{}", value.as_int);

    case ValueTag::Int:
      return fmt::format("{}", value.as_int);

    case ValueTag::Bool:
      return fmt::format("{}", value.as_bool);

    case ValueTag::Char:
      return fmt::format("{}", value.as_char);

    case ValueTag::InstrRef:
      return FormatInstrRef(value.as_ref.to_instr);

    case ValueTag::StackRef:
      return fmt::format("{}", value.as_ref.to_data);

    case ValueTag::HeapRef:
      return "HeapRef";

    case ValueTag::StaticRef:
      return "StaticRef";

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

std::string FormatPrimitiveValue(PrimitiveValue value) {
  return fmt::format("Value [tag:{}, value:{}]", FormatValueTag(value.tag),
                     FormatValue(value));
}

}  // namespace vm::rt
