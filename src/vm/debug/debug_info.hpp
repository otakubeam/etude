#pragma once

#include <lex/location.hpp>

#include <optional>

namespace vm::debug {

struct DebuggerInstr {
  std::string var_name{};
  std::string type_name{};
};

// Additional information along with instruction
struct DebugInfo {
  lex::Location location;
  std::optional<DebuggerInstr> dbg_instr;
  uint16_t executable_location = 0;

  std::string Format() {
    return fmt::format("{} {} {}", location.Format(), executable_location,
                       dbg_instr.value_or(DebuggerInstr{}).type_name +
                           dbg_instr.value_or(DebuggerInstr{}).var_name);
  }
};

}  // namespace vm::debug
