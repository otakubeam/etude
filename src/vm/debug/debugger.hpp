#pragma once

#include <vm/debug/stack_printer.hpp>
#include <vm/debug/disassember.hpp>

#include <vm/interpreter.hpp>

namespace vm::debug {

class Debugger : public BytecodeInterpreter {
 public:
  bool Step() {
    auto instr = GetNextInstruction();
    fmt::print("Current instruction {}: {}", rt::FormatInstrRef(ip_),
               Disassembler::FormatInstruction(instr));
    try {
      RunFor(1);
    } catch (rt::PrimitiveValue ret) {
      return_ = ret;
      return false;
    }

    fmt::print("{}\n\n", printer_.ToDot());
    printer_.Print();
    return true;
  }

  rt::PrimitiveValue StepToTheEnd() {
    while (Step())
      ;
    return return_.value();
  }

 private:
  StackPrinter printer_{stack_};
  std::optional<rt::PrimitiveValue> return_;
};

}  // namespace vm::debug
