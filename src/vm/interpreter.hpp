#pragma once

#include <vm/memory/vm_memory.hpp>

#include <vm/instr_type.hpp>

#include <vm/decoder.hpp>

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

  uint8_t DecodeExecute(uint8_t* instr) {
    switch (Decoder::DecodeType(instr)) {
      case InstrType::PUSH_VALUE: {
        auto value = Decoder::DecodeValue(instr);
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
        return 1;
      }

      case InstrType::CALL_FN: {
        // Save ip
        memory_.GetStack().Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::InstrRef,
            .as_ref = rt::Reference{.to_instr = ip_},
        });

        // Save old fp, move fp and sp
        memory_.GetStack().PrepareCallframe();

        // Jump to the new function
        auto ref = Decoder::DecodeReference(instr);
        ip_ = *ref;

        // The next step is 0 (so we start with the first instr)
        return 0;
      }

      case InstrType::RET_FN: {
        rax_ = memory_.GetStack().Pop();

        memory_.GetStack().Ret();

        auto saved_ip = memory_.GetStack().Pop();
        FMT_ASSERT(saved_ip.tag == rt::ValueTag::InstrRef,
                   "Found garbage instead of saved ip\n");

        ip_ = saved_ip.as_ref.to_instr;

        // Start from the saved next instruction
        return 1 + sizeof(rt::InstrReference);
      }

      case InstrType::FIN_CALL: {
        auto count = Decoder::DecodeByte(instr);

        memory_.GetStack().PopCount(count);

        // Safety: the value must be present in rax after the call
        memory_.GetStack().Push(std::move(rax_));

        return 2;
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
    FMT_ASSERT(false, "Unreachable!");
  }

 private:
  vm::rt::InstrReference ip_;

  // Return register
  vm::rt::PrimitiveValue rax_{};

  vm::memory::VmMemory memory_{4096, 1024};
};

}  // namespace vm
