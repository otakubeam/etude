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

std::string FormatPrimitiveValue(PrimitiveValue value) {
  auto result = fmt::format("Value [tag:{}, ", FormatValueTag(value.tag));

  switch (value.tag) {
    case ValueTag::Int:
      result.append(fmt::format("value {}]", value.as_int));
      return result;
    case ValueTag::Bool:
      result.append(fmt::format("value {}]", value.as_bool));
      return result;
    case ValueTag::Unit:
      result.append(fmt::format("value unit({})]", value.as_int));
      return result;
    case ValueTag::Char:
      result.append(fmt::format("value char({})]", value.as_char));
      return result;
    case ValueTag::HeapRef:
      return "HeapRef";
    case ValueTag::StackRef:
      result.append(fmt::format("value ({})]", value.as_ref.to_data));
      return result;
    case ValueTag::InstrRef:
      result.append(fmt::format("value ({}, {})]",
                                value.as_ref.to_instr.chunk_no,
                                value.as_ref.to_instr.instr_no));
      return result;
    case ValueTag::StaticRef:
      return "StaticRef";
    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

}  // namespace vm::rt
