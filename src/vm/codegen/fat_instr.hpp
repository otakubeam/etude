#pragma once

#include <vm/debug/debug_info.hpp>

#include <vm/rt/primitive.hpp>

#include <vm/instr_type.hpp>

#include <fmt/core.h>

#include <cstdlib>

namespace vm::codegen {

//////////////////////////////////////////////////////////////////////

// Fat instructions are used for compilation

struct FatInstr {
  InstrType type{};

  u_int8_t arg = 0;

  int16_t offset = 0;

  std::string fn_name{};

  rt::PrimitiveValue value{};

  debug::DebugInfo debug_info{};

  static auto MakePushInt(int value) -> vm::codegen::FatInstr;
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm::codegen
