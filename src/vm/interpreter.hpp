#pragma once

#include <vm/memory/vm_memory.hpp>

#include <vm/instr_type.hpp>

#include <vm/decoder.hpp>

#include <optional>

namespace vm {

class BytecodeInterpreter {
 public:
  void Load(ElfFile executable);

  void RunFor(size_t count);

 private:
  uint8_t* GetNextInstruction(uint8_t step);

  uint8_t DecodeExecute(uint8_t* instr);

 private:
  vm::rt::InstrReference ip_;

  // Return register
  vm::rt::PrimitiveValue rax_{};

  vm::memory::VmMemory memory_{4096, 1024};
};

}  // namespace vm
