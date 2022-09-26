#include <vm/interpreter.hpp>

#include <vm/debug/disassember.hpp>

namespace vm {

void BytecodeInterpreter::Load(ElfFile executable) {
  ip_ = executable.FindEntryPoint().value();
  memory_.Load(std::move(executable));
}

void BytecodeInterpreter::RunFor(size_t count) {
  uint8_t step = 0;
  for (size_t i = 0; i < count; i++) {
    auto instr = GetNextInstruction(step);
    step = DecodeExecute(instr);
    // debug::Disassembler::Decode(instr);
  }
}

auto BytecodeInterpreter::GetNextInstruction(uint8_t step) -> uint8_t* {
  ip_.instr_no += step;

  // MakeInstrAccess(ip_);
  return memory_.AccessMemory(memory::MemAccess{
      .reference =
          {
              .tag = rt::ValueTag::InstrRef,
              .as_ref =
                  rt::Reference{
                      .to_instr = ip_,
                  },
          },
  });
}

uint8_t BytecodeInterpreter::DecodeExecute(uint8_t* instr) {
  switch (Decoder::DecodeType(instr)) {
    case InstrType::PUSH_VALUE: {
      auto value = Decoder::DecodeValue(instr);
      memory_.GetStack().Push(*value);
      return 1 + sizeof(rt::PrimitiveValue);
    }

    case InstrType::CALL_FN: {
      ip_.instr_no += 1 + sizeof(rt::InstrReference);

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

    case InstrType::LOAD: {
      auto addr = memory_.GetStack().Pop();

      switch (addr.tag) {
        case rt::ValueTag::HeapRef:
        case rt::ValueTag::StaticRef:
        case rt::ValueTag::StackRef: {
          auto loc = memory_.AccessMemory({.reference = addr, .store = false});
          memory_.GetStack().Push(*(rt::PrimitiveValue*)loc);
          break;
        }
        case rt::ValueTag::InstrRef:
          FMT_ASSERT(false, "Not a value\n");
        default:
          FMT_ASSERT(false, "Not a reference\n");
      }

      return 1;
    }

    case InstrType::STORE: {
      auto addr = memory_.GetStack().Pop();
      auto value = memory_.GetStack().Pop();

      switch (addr.tag) {
        case rt::ValueTag::HeapRef:
        case rt::ValueTag::StaticRef:
        case rt::ValueTag::StackRef: {
          auto loc = memory_.AccessMemory({.reference = addr, .store = true});
          memcpy(loc, &value, sizeof(value));
          break;
        }
        case rt::ValueTag::InstrRef:
          FMT_ASSERT(false, "Not a value\n");
        default:
          FMT_ASSERT(false, "Not a reference\n");
      }

      return 1;
    }

    case InstrType::INDIRECT_CALL: {
      ip_.instr_no += 1;

      // Save ip
      memory_.GetStack().Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::InstrRef,
          .as_ref = rt::Reference{.to_instr = ip_},
      });

      // Save old fp, move fp and sp
      memory_.GetStack().PrepareCallframe();

      auto mem_ref = memory_.GetStack().Pop();
      FMT_ASSERT(mem_ref.tag == rt::ValueTag::InstrRef,
                 "Calling to nonexecutable memory");

      ip_ = mem_ref.as_ref.to_instr;

      return 0;
    }

    case InstrType::NATIVE_CALL: {
      auto native_no = Decoder::DecodeByte(instr);
      (void)native_no;
      // TODO: make a call into the NativeTable
      FMT_ASSERT(false, "Unimplemented!");
    }

    case InstrType::RET_FN: {
      rax_ = memory_.GetStack().Pop();

      memory_.GetStack().Ret();

      auto saved_ip = memory_.GetStack().Pop();
      FMT_ASSERT(saved_ip.tag == rt::ValueTag::InstrRef,
                 "Found garbage instead of saved ip\n");

      ip_ = saved_ip.as_ref.to_instr;

      // Start from the saved next instruction
      return 0;
    }

    case InstrType::FIN_CALL: {
      auto count = Decoder::DecodeByte(instr);

      memory_.GetStack().PopCount(count);

      // Safety: the value must be present in rax after the call
      memory_.GetStack().Push(std::move(rax_));

      return 2;
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

    case InstrType::SUBTRACT: {
      auto rhs = memory_.GetStack().Pop().as_int;
      auto lhs = memory_.GetStack().Pop().as_int;

      memory_.GetStack().Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Int,
          .as_int = lhs - rhs,
      });

      return 1;
    }

    case InstrType::CMP_EQ: {
      auto a = memory_.GetStack().Pop();
      auto b = memory_.GetStack().Pop();

      memory_.GetStack().Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Bool,
          .as_bool = a.tag == b.tag && a.as_int == b.as_int,
      });

      return 1;
    }

    case InstrType::CMP_LESS: {
      auto a = memory_.GetStack().Pop();
      auto b = memory_.GetStack().Pop();

      memory_.GetStack().Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Bool,
          .as_bool = a.tag == b.tag && a.as_int < b.as_int,
      });

      return 1;
    }

    case InstrType::PUSH_FALSE: {
      memory_.GetStack().Push({.tag = rt::ValueTag::Bool, .as_bool = false});
      return 1;
    }

    case InstrType::PUSH_TRUE: {
      memory_.GetStack().Push({.tag = rt::ValueTag::Bool, .as_bool = true});
      return 1;
    }

    case InstrType::POP_STACK: {
      memory_.GetStack().Pop();
      return 1;
    }

    case InstrType::JUMP: {
      auto jump_offset = Decoder::DecodeOffset(instr);
      ip_.instr_no += jump_offset;
      return 0;
    }

    case InstrType::JUMP_IF_FALSE: {
      auto condition = Decoder::DecodeValue(instr);
      auto jump_offset = Decoder::DecodeOffset(instr);

      if (condition) {
        return 1 + sizeof(rt::PrimitiveValue) + sizeof(int16_t);
      }

      ip_.instr_no += jump_offset;
      return 0;
    }

    case InstrType::GET_ARG:
    case InstrType::GET_LOCAL:
      FMT_ASSERT(false, "Unimplemented!");
  }

  FMT_ASSERT(false, "Unreachable!");
}

}  // namespace vm
