#pragma once

#include <vm/rt/primitive.hpp>

#include <vm/instr.hpp>

#include <cstdlib>
#include <vector>

namespace vm {

//////////////////////////////////////////////////////////////////////

struct ExecutableChunk {
  std::vector<Instr> instructions;
  std::vector<rt::PrimitiveValue> attached_vals;

  void Print() {
    fmt::print("[!] Executable chunk:\n");

    for (auto i : instructions) {
      fmt::print("\t[.] Instruction: {}\n", PrintInstr(i));
    }

    fmt::print("\n\n");

    for (auto a : attached_vals) {
      fmt::print("\t[-] Value: {}\n", a.as_int);
    }
  }
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
