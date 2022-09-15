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

    auto idx = 0;
    for (auto i : instructions) {
      fmt::print("\t[.] Instruction {}:\t{}\n", idx++, PrintInstr(i));
    }

    fmt::print("\n\n");

    idx = 0;
    for (auto a : attached_vals) {
      fmt::print("\t[-] Value {}:\t{}\n", idx++, a.as_int);
    }
  }
};

//////////////////////////////////////////////////////////////////////

}  // namespace vm
