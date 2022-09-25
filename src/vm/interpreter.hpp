#pragma once

#include <vm/memory/vm_memory.hpp>

#include <vm/instr_type.hpp>

#include <optional>

namespace vm {

class BytecodeInterpreter {
 public:
  void Load(ElfFile executable) {
    ip_ = executable.FindEntryPoint().value();
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
    switch (DecodeType(instr)) {
      case InstrType::PUSH_VALUE: {
        auto value = DecodeValue(instr);
        memory_.GetStack().Push(*value);
        return 1 + sizeof(rt::PrimitiveValue);
      }

      case InstrType::ADD: {
        auto a = memory_.GetStack().Pop().as_int;
        auto b = memory_.GetStack().Pop().as_int;
        memory_.GetStack().Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::Int,
            .as_int = a + b,
        });
        break;
      }

      case InstrType::CALL_FN: {
        // Point to the next instruction
        ip_.instr_no += 1 + sizeof(rt::PrimitiveValue);

        // Save ip
        memory_.GetStack().Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::InstrRef,
            .as_ref = rt::Reference{.to_instr = ip_},
        });

        // Save old fp, move fp and sp
        memory_.GetStack().PrepareCallframe();

        // Jump to the new function
        auto ref = DecodeReference(instr);
        ip_ = *ref;

        // The next step is 0 (so we start with the first instr)
        return 0;
      }

      case InstrType::RET_FN: {
        // TODO: what if we are at the top level?
        memory_.GetStack().Ret();
        // TODO: return value
        break;
      }

      case InstrType::PUSH_FALSE:;
      case InstrType::PUSH_TRUE:;
      case InstrType::NATIVE_CALL:;
      case InstrType::INDIRECT_CALL:;
      case InstrType::CMP_EQ:;
      case InstrType::CMP_LESS:;
      case InstrType::POP_STACK:;
      default:;
    }
    std::abort();
  }

  auto DecodeType(uint8_t*& instr) -> InstrType {
    return static_cast<InstrType>(*instr++);
  }

  auto DecodeValue(uint8_t*& instr) -> rt::PrimitiveValue* {
    auto value = (rt::PrimitiveValue*)instr;
    instr += sizeof(rt::PrimitiveValue);
    return value;
  };

  auto DecodeReference(uint8_t*& instr) -> rt::InstrReference* {
    auto value = (rt::InstrReference*)instr;
    instr += sizeof(rt::InstrReference);
    return value;
  };

 private:
  vm::rt::InstrReference ip_;

  vm::memory::VmMemory memory_{4096, 1024};
};

}  // namespace vm
