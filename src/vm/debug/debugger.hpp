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

    static std::ofstream stream{"dots/raw"};
    stream << ToDot();

    try {
      RunFor(1);
    } catch (rt::PrimitiveValue ret) {
      return_ = ret;
      return false;
    }

    printer_.Print();

    return true;
  }

  std::string ToDot() {
    return fmt::format(
        "digraph G {{ {}\n inst [label=\"{:<80}\"]; \n inst -> sp; \n {} }} "
        "\n",  //
        printer_.ToDot(), FormatCurrentInstruction(), FormatHeapPtrs());
  }

  auto GetHeapPtrs() {
    std::vector<uint32_t> heap_ptrs;
    rt::PrimitiveValue* it = (rt::PrimitiveValue*)memory_.GetStackArea();

    fmt::print("Want to get heap ptrs");

    for (size_t i = 0; &it[i] <= &stack_.Top(); i++) {
      fmt::print("Tag[{}]: {}\n", i, (uint8_t)it[i].tag);
      if (it[i].tag == rt::ValueTag::HeapRef) {
        heap_ptrs.push_back(i);
      }
    }

    return heap_ptrs;
  }

  std::string FormatHeapPtrs() {
    fmt::memory_buffer buf;

    auto heap_ptrs = GetHeapPtrs();

    for (auto ptr : heap_ptrs) {
      auto [loc, size] = *memory_.LookupSize(ptr);

      std::string structure = "";
      for (size_t i = 0; i < size; i++) {
        structure += fmt::format("<td port='{}'>{}</td> ", loc + i, 123);
      }

      fmt::format_to(std::back_inserter(buf),
                     "heap_{} [label=<<table>\n <tr>{} </tr></table>>] \n", loc,
                     structure);
    }

    return fmt::to_string(buf);
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
