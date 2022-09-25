#include <vm/codegen/fat_instr.hpp>

namespace vm::codegen {

auto FatInstr::MakePushInt(int value) -> vm::codegen::FatInstr {
  return FatInstr{
      .type = InstrType::PUSH_VALUE,
      .value =
          {
              .tag = rt::ValueTag::Int,
              .as_int = value,
          },
  };
}

}  // namespace vm::codegen
