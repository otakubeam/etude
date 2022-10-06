#pragma once

#include <vm/debug/stack_printer.hpp>
#include <vm/debug/disassember.hpp>

#include <vm/interpreter.hpp>

namespace vm::debug {

class Debugger : public BytecodeInterpreter {
 public:
  void Step() {
    auto instr = GetNextInstruction();
    fmt::print("Current instruction {}: {}", rt::FormatInstrRef(ip_),
               Disassembler::FormatInstruction(instr));

    RunFor(1);
    printer.Print();
  }

 private:
  StackPrinter printer{stack_};
};

}  // namespace vm::debug
