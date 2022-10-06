#include <vm/interpreter.hpp>

#include <vm/debug/disassember.hpp>

namespace vm {

void BytecodeInterpreter::Load(ElfFile executable) {
  ip_ = executable.FindEntryPoint().value();
  memory_.Load(std::move(executable));
}

rt::PrimitiveValue BytecodeInterpreter::RunToTheEnd() {
  try {
    RunFor(100500);
  } catch (rt::PrimitiveValue return_value) {
    return return_value;
  }
  FMT_ASSERT(false, "Unreachable!");
}

void BytecodeInterpreter::RunFor(size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (auto instr = GetNextInstruction()) {
      ip_.instr_no += DecodeExecute(instr);
    }
  }
}

auto BytecodeInterpreter::GetNextInstruction() -> uint8_t* {
  // MakeInstrAccess(ip_);
  return memory_.AccessMemory({.reference = {
                                   .tag = rt::ValueTag::InstrRef,
                                   .as_ref = {.to_instr = ip_},
                               }});
}

uint8_t BytecodeInterpreter::DecodeExecute(uint8_t* instr) {
  switch (Decoder::DecodeType(instr)) {
    case InstrType::PUSH_VALUE: {
      auto value = Decoder::DecodeValue(instr);
      stack_.Push(*value);

      return 1 + sizeof(rt::PrimitiveValue);
    }

    case InstrType::CALL_FN: {
      // Remember to move the ip the next time we get to this function
      ip_.instr_no += 1 + sizeof(rt::InstrReference);

      // Save ip
      stack_.Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::InstrRef,
          .as_ref = {.to_instr = ip_},
      });

      // Save old fp, move fp and sp
      stack_.PrepareCallframe();

      // Jump to the new function
      auto ref = Decoder::DecodeReference(instr);
      ip_ = *ref;

      // The next step is 0 (so we start with the first instr)
      return 0;
    }

    case InstrType::LOAD: {
      auto addr = stack_.Pop();

      switch (addr.tag) {
        case rt::ValueTag::HeapRef:
        case rt::ValueTag::StaticRef:
        case rt::ValueTag::StackRef: {
          auto src = memory_.AccessMemory({.reference = addr, .store = false});
          stack_.Push(*(rt::PrimitiveValue*)src);
          // memcpy(&stack_.Top() + 1, src, sizeof(rt::PrimitiveValue) *
          // store_words);
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
      auto addr = stack_.Pop();
      auto store_words = Decoder::DecodeByte(instr);

      switch (addr.tag) {
        case rt::ValueTag::StackRef: {
          auto dst = memory_.AccessMemory({.reference = addr, .store = true});
          //
          // Due to semantics of Top (pointing to the first *real* element)
          // neeed to subtract 1 here
          //
          auto src_zone_start = &stack_.Top() - (store_words - 1);
          memcpy(dst, src_zone_start, sizeof(rt::PrimitiveValue) * store_words);

          stack_.PopCount(store_words);
          break;
        }

        case rt::ValueTag::HeapRef:
        case rt::ValueTag::StaticRef:
        case rt::ValueTag::InstrRef:
          FMT_ASSERT(false, "Not a value\n");
        default:
          FMT_ASSERT(false, "Not a reference\n");
      }

      return 2;
    }

    case InstrType::INDIRECT_CALL: {
      ip_.instr_no += 1;

      // Save ip
      stack_.Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::InstrRef,
          .as_ref = {.to_instr = ip_},
      });

      // Save old fp, move fp and sp
      stack_.PrepareCallframe();

      auto mem_ref = stack_.Pop();
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
      rax_ = stack_.Pop();

      if (!stack_.Ret()) {
        throw rax_;  // ~sys_exit
      }

      auto saved_ip = stack_.Pop();
      FMT_ASSERT(saved_ip.tag == rt::ValueTag::InstrRef,
                 "Found garbage instead of saved ip\n");

      ip_ = saved_ip.as_ref.to_instr;

      // Start from the saved next instruction
      return 0;
    }

    case InstrType::FIN_CALL: {
      auto count = Decoder::DecodeByte(instr);

      stack_.PopCount(count);

      // Safety: the value must be present in rax after the call
      stack_.Push(std::move(rax_));

      return 2;
    }

    case InstrType::ADD: {
      auto a = stack_.Pop().as_int;
      auto b = stack_.Pop().as_int;

      stack_.Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Int,
          .as_int = a + b,
      });

      return 1;
    }

    case InstrType::SUBTRACT: {
      auto rhs = stack_.Pop().as_int;
      auto lhs = stack_.Pop().as_int;

      stack_.Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Int,
          .as_int = lhs - rhs,
      });

      return 1;
    }

    case InstrType::CMP_EQ: {
      auto a = stack_.Pop();
      auto b = stack_.Pop();

      stack_.Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Bool,
          .as_bool = a.tag == b.tag && a.as_int == b.as_int,
      });

      return 1;
    }

    case InstrType::CMP_LESS: {
      auto a = stack_.Pop();
      auto b = stack_.Pop();

      stack_.Push(rt::PrimitiveValue{
          .tag = rt::ValueTag::Bool,
          .as_bool = a.tag == b.tag && a.as_int < b.as_int,
      });

      return 1;
    }

    case InstrType::PUSH_FALSE:
      stack_.Push({.tag = rt::ValueTag::Bool, .as_bool = false});
      return 1;

    case InstrType::PUSH_TRUE:
      stack_.Push({.tag = rt::ValueTag::Bool, .as_bool = true});
      return 1;

    case InstrType::PUSH_UNIT:
      stack_.Push({.tag = rt::ValueTag::Unit, .as_char = '\0'});
      return 1;

    case InstrType::PUSH_FP:
      stack_.Push({.tag = rt::ValueTag::StackRef,
                   .as_ref = {.to_data = stack_.GetFp()}});
      return 1;

    case InstrType::ADD_ADDR: {
      auto addition = Decoder::DecodeOffset(instr);
      stack_.Top().as_ref.to_data += addition;
      FMT_ASSERT(stack_.Top().tag == rt::ValueTag::StackRef,
                 "Can only add offset to addresses");
      return 3;
    }

    case InstrType::POP_STACK:
      stack_.Pop();
      return 1;

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

    case InstrType::GET_AT_FP: {
      auto offset = Decoder::DecodeOffset(instr);
      stack_.GetAtFp(offset);
      return 3;
    }
  }

  FMT_ASSERT(false, "Unreachable!");
}

}  // namespace vm
