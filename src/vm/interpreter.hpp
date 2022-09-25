#pragma once

#include <vm/memory/vm_memory.hpp>

#include <vm/instr_type.hpp>

#include <optional>

namespace vm {

class BytecodeInterpreter {
 public:
  void Load(ElfFile executable) {
    ip_.chunk_no = executable.FindMain().value();

    memory_.Load(std::move(executable));
  }

  void RunFor(size_t count) {
    uint8_t step = 0;
    for (size_t i = 0; i < count; i++) {
      auto instr = GetNextInstruction(step);
      step = DecodeExecute(instr);
    }
  }

 private:
  auto GetNextInstruction(uint8_t step) -> uint8_t* {
    ip_.instr_no += step;

    return memory_.AccessMemory(memory::MemAccess{
        .mem_ref = {.to_instr = ip_},
        .type = rt::ValueTag::InstrRef,
    });
  }

  // The logic about decoding should be right here
  uint8_t DecodeExecute(uint8_t* instr) {
    switch (DecodeType(instr++)) {
      case InstrType::PUSH_VALUE: {
        // Later: call this decode value or something
        auto value = (rt::PrimitiveValue*)instr;
        instr += sizeof(rt::PrimitiveValue);

        fmt::print("Tag: {}, value: {}\n", (uint8_t)value->tag, value->as_int);

        memory_.GetStack().Push(*value);
        return 1 + sizeof(rt::PrimitiveValue);
      }

      case InstrType::PUSH_FALSE:;
      case InstrType::PUSH_TRUE:;
      case InstrType::ADD: {
        auto a = memory_.GetStack().Pop().as_int;
        auto b = memory_.GetStack().Pop().as_int;
        memory_.GetStack().Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::Int,
            .as_int = a + b,
        });
        // Print it
        memory_.GetStack();
        break;
      }
      case InstrType::CALL_FN:;
      case InstrType::POP_STACK:;

      case InstrType::RET_FN: {
        // TODO: what if we are at the top level?
      }

      case InstrType::CMP_EQ:;
      case InstrType::CMP_LESS:;
      case InstrType::INDIRECT_CALL:;
      case InstrType::NATIVE_CALL:;
      default:
        break;
    }
    return 1;
  }

  auto DecodeType(uint8_t* instr) -> InstrType {
    return static_cast<InstrType>(*instr);
  }

 private:
  vm::rt::InstrReference ip_;

  vm::memory::VmMemory memory_{4096, 1024};
};

}  // namespace vm
