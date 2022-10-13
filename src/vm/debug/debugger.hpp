#pragma once

#include <vm/debug/stack_printer.hpp>
#include <vm/debug/disassember.hpp>

#include <vm/interpreter.hpp>

#include <fstream>

namespace vm::debug {

class Debugger : public BytecodeInterpreter {
 public:
  bool Step() {
    if (auto instr = memory_.program_text_->DIEs_.at(ip_.chunk_no)
                         .at(ip_.instr_no)
                         .dbg_instr) {
      printer_.AnnotateSlot(*instr);
    }

    auto instr = GetNextInstruction();
    fmt::print("Current instruction {}: {}", rt::FormatInstrRef(ip_),
               Disassembler::FormatInstruction(instr));
    try {
      RunFor(1);
    } catch (rt::PrimitiveValue ret) {
      return_ = ret;
      return false;
    }

    printer_.Print();

    static std::ofstream stream{"dots/raw"};
    stream << ToDot();

    return true;
  }

  std::string ToDot() {
    return fmt::format(
        "digraph G {{ {}\n inst [label=\"{:<80}\"]; \n inst -> sp; \n }} "
        "\n",  //
        printer_.ToDot(), FormatCurrentInstruction());
  }

  std::string FormatCurrentInstruction() {
    auto instr = GetNextInstruction();
    return fmt::format("{}: {}", rt::FormatInstrRef(ip_),
                       Disassembler::FormatInstruction(instr));
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
