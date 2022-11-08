#pragma once

#include <vm/rt/primitive.hpp>
#include <vm/instr_type.hpp>

namespace vm {

struct Decoder {
  static auto DecodeByte(uint8_t*& instr) -> uint8_t {
    return *instr++;
  }

  static auto DecodeType(uint8_t*& instr) -> InstrType {
    return static_cast<InstrType>(DecodeByte(instr));
  }

  static auto DecodeOffset(uint8_t*& instr) -> int16_t {
    int16_t value{};
    memcpy(&value, instr, sizeof(int16_t));
    instr += sizeof(int16_t);
    return value;
  }

  static auto DecodeValue(uint8_t*& instr) -> rt::PrimitiveValue {
    rt::PrimitiveValue value{};
    memcpy(&value, instr, sizeof(rt::PrimitiveValue));
    instr += sizeof(rt::PrimitiveValue);
    return value;
  };

  static auto DecodeReference(uint8_t*& instr) -> rt::InstrReference {
    rt::InstrReference value{};
    memcpy(&value, instr, sizeof(rt::InstrReference));
    instr += sizeof(rt::InstrReference);
    return value;
  };
};

}  // namespace vm
