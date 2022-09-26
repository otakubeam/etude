#pragma once

#include <lex/token.hpp>

#include <fmt/format.h>

#include <cstdint>
#include <variant>
#include <string>

namespace vm::rt {

//////////////////////////////////////////////////////////////////////

enum class ValueTag : uint8_t {
  Int,
  Bool,
  Unit,
  Char,

  HeapRef,
  StackRef,
  InstrRef,
  StaticRef,
};

std::string FormatValueTag(ValueTag tag);

static_assert(sizeof(ValueTag) == 1);

//////////////////////////////////////////////////////////////////////

struct InstrReference {
  uint16_t chunk_no = 0;
  uint16_t instr_no = 0;
};

std::string FormatInstrRef(InstrReference instr_ref);

static_assert(sizeof(InstrReference) == 4);

//////////////////////////////////////////////////////////////////////

struct Reference {
  union {
    uint32_t to_data;
    InstrReference to_instr;
  };
};

static_assert(sizeof(Reference) == 4);

//////////////////////////////////////////////////////////////////////

// #pragma pack(1)
struct PrimitiveValue {
  ValueTag tag;
  union {
    int as_int;
    bool as_bool;
    char as_char;
    Reference as_ref;
  };
};
// #pragma pack(0)

// static_assert(sizeof(PrimitiveValue) == 5);

//////////////////////////////////////////////////////////////////////

std::string FormatPrimitiveValue(PrimitiveValue value);

//////////////////////////////////////////////////////////////////////

}  // namespace vm::rt
