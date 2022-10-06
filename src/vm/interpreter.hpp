#pragma once

#include <vm/memory/vm_memory.hpp>
#include <vm/memory/stack_v2.hpp>

#include <vm/instr_type.hpp>

#include <vm/decoder.hpp>

#include <optional>

namespace vm::debug {
class Debugger;
}

namespace vm {

class BytecodeInterpreter {
 public:
  // friend class debug::Debugger;

  void Load(ElfFile executable);

  void RunFor(size_t count);

 private:
  uint8_t DecodeExecute(uint8_t* instr);

 protected:
  uint8_t* GetNextInstruction();

 protected:
  rt::InstrReference ip_;

  // Return register
  rt::PrimitiveValue rax_{};

  memory::VmMemory memory_{4096, 1024};

  memory::VmStack stack_{memory_};
};

}  // namespace vm
