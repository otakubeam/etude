#pragma once

#include <lex/token.hpp>

#include <fmt/format.h>

#include <cstdint>
#include <variant>
#include <string>

namespace vm::rt {

//////////////////////////////////////////////////////////////////////

enum class ValueTag {
  Int,
  Bool,
  Unit,
  Char,

  HeapRef,
  StackRef,
  InstrRef,
  StaticRef,
};

//////////////////////////////////////////////////////////////////////

struct InstrReference {
  uint16_t chunk_no = 0;
  uint16_t instr_no = 0;
};

//////////////////////////////////////////////////////////////////////

struct Reference {
  union {
    uint32_t to_data;
    InstrReference to_instr;
  };
};

//////////////////////////////////////////////////////////////////////

struct PrimitiveValue {
  ValueTag tag;
  union {
    int as_int;
    bool as_bool;
    char as_char;
    Reference as_ref;
  };
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm::rt
