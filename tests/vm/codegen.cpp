#include <vm/codegen/compiler.hpp>

#include <vm/debug/disassember.hpp>
#include <vm/debug/debugger.hpp>

#include <vm/instr_translator.hpp>

#include <vm/interpreter.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codgen:simple", "[vm:codgen]") {
  char stream[] =
      "   fun main() Int {     "
      "      var a = 5;       "
      "      var b = &a;      "
      "      *b = 6;          "
      "      *b               "
      "   }                   ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};
  auto stmt = p.ParseFunDeclStatement();

  types::check::TypeChecker tchk;
  tchk.Eval(stmt);

  vm::codegen::Compiler compiler;
  auto elf = compiler.Compile(stmt);

  vm::debug::Disassembler d;
  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(std::move(elf));

  for (auto i = 0; i < 16; i++) {
    debugger.Step();
  }
}

//////////////////////////////////////////////////////////////////////
