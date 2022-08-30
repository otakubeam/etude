#pragma once

#include <vm/stack.hpp>
#include <vm/chunk.hpp>

#include <optional>

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

        break;
      }

      case InstrType::RET_FN: {
        // Obtain the return value
        eax = stack_.Pop();

        stack_.Ret();

        // Restore ip
        ip_ = stack_.Pop().as_int;

        // Then the caller must clean up

        break;
      }

      case InstrType::CALL_FN: {
        // Note: Args have been placed

        // Push IP onto the stack
        stack_.Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::Int,
            .as_int = (int)ip_,
        });

        // Create a new call frame
        stack_.PrepareCallframe();

        // Jmp into the function
        ip_ = ReadWord(instruction);

        break;
      }
    }
  }

  int Interpret(ExecutableChunk* chunk_main) {
    current_ = chunk_main;

    for (auto& i : current_->instructions) {
      Interpret(i);
    }

    // Exit code
    return stack_.Pop().as_int;
  }

 private:
  // Instruction pointer
  size_t ip_ = 0;

  vm::VmStack stack_;

  using Retval = std::optional<rt::PrimitiveValue>;
  // Return value (makes life easier)
  Retval eax;

  ExecutableChunk* current_ = nullptr;
  std::vector<ExecutableChunk> chunks;
};

}  // namespace vm
