#pragma once

#include <vm/stack.hpp>
#include <vm/chunk.hpp>

namespace vm {

class BytecodeInterpreter {
 public:
  void Interpret(Instr instruction) {
    switch (instruction.type) {
      case InstrType::PUSH_STACK: {
        size_t index = instruction.arg1;
        stack_.Push(current_->attached_vals[index]);
        break;
      }

      case InstrType::POP_STACK: {
        stack_.Pop();
      }
    }
  }

  int Interpret(ExecutableChunk* chunk_main) {
    current_ = chunk_main;

    for (auto& i : chunk_main->instructions) {
      Interpret(i);
    }

    // Exit code
    return stack_.Pop().as_int;
  }

 private:
  vm::VmStack stack_;

  ExecutableChunk* current_;
  std::vector<ExecutableChunk> chunks;
};

}  // namespace vm
