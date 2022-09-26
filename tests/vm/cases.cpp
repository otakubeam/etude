#include <vm/debug/disassember.hpp>

#include <vm/instr_translator.hpp>

#include <vm/interpreter.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: retval", "[vm]") {
  ////////////////////////////////////////////////////////////////////

  vm::InstrTranslator assembler_f{"f"};
  assembler_f.TranslateInstruction(vm::codegen::FatInstr::MakePushInt(100));
  assembler_f.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::CALL_FN,
      .fn_name = "f",
  });
  assembler_f.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::RET_FN,
  });
  auto elf_f = std::move(assembler_f).Finalize();

  vm::debug::Disassembler disasm;
  disasm.Disassemble(elf_f);

  ////////////////////////////////////////////////////////////////////

  vm::InstrTranslator assembler{"main"};
  assembler.TranslateInstruction(vm::codegen::FatInstr::MakePushInt(100));
  assembler.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::CALL_FN,
      .fn_name = "f",
  });
  assembler.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::FIN_CALL,
  });
  assembler.TranslateInstruction(vm::codegen::FatInstr{
      .type = vm::InstrType::ADD,
  });
  auto elf = std::move(assembler).Finalize();

  disasm.Disassemble(elf);

  ////////////////////////////////////////////////////////////////////

  fmt::print("\n\nNow let's link these files!\n\n");

  elf += std::move(elf_f);

  disasm.Disassemble(elf);

  vm::BytecodeInterpreter interpreter;

  interpreter.Load(std::move(elf));

  interpreter.RunFor(16);
}

//////////////////////////////////////////////////////////////////////
