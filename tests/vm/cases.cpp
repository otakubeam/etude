#include <vm/instr_translator.hpp>

#include <vm/interpreter.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: retval", "[vm]") {
  vm::InstrTranslator assembler{"main"};

  assembler.TranslateInstruction(vm::codegen::FatInstr::MakePushInt(100));
  assembler.TranslateInstruction(vm::codegen::FatInstr::MakePushInt(1));
  assembler.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::ADD,
  });
  assembler.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::RET_FN,
  });

  auto elf = std::move(assembler).Finalize();

  vm::BytecodeInterpreter interpreter;
  interpreter.Load(std::move(elf));

  interpreter.RunFor(4);
}

//////////////////////////////////////////////////////////////////////
