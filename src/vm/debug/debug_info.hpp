#pragma once

#include <lex/location.hpp>

#include <optional>

namespace vm::debug {

struct DebuggerInstr {
  std::string var_name;

  // Maybe
  // std::string type_name;
};

// Additional information along with instruction
struct DebugInfo {
  lex::Location location;
  std::optional<DebuggerInstr> dbg_instr;
};

}  // namespace vm::debug
