#pragma once

#include <vm/debug/stack_printer.hpp>
#include <vm/debug/disassember.hpp>

#include <vm/interpreter.hpp>

#include <fstream>

namespace vm::debug {

////////////////////////////////////////////////////////////////////

class Debugger : public BytecodeInterpreter {
 public:
  bool Step();

  rt::PrimitiveValue StepToTheEnd();

 private:
  std::string ToDot();

  std::string FormatCurrentInstruction();

  //////////////////////////////////////////////////////////////////

  struct HeapPrinter {
    std::string FormatHeapPtrs();

    void FormatStrucutre(auto ptr);

    void FormatSourceNode(auto loc, auto i);

    void FormatDestinationNode(const auto& value);

    Debugger& this_debugger;
    fmt::memory_buffer buf{};
  };

  auto GetHeapPtrs() -> std::vector<uint32_t>;

  //////////////////////////////////////////////////////////////////

 private:
  StackPrinter printer_{stack_};

  std::optional<rt::PrimitiveValue> return_;
};

////////////////////////////////////////////////////////////////////

}  // namespace vm::debug
