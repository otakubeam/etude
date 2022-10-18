#include <types/check/type_checker.hpp>
#include <types/check/type_error.hpp>

#include <vm/codegen/compiler.hpp>
#include <vm/debug/debugger.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <fmt/color.h>

#include <fstream>

void RunPhony(vm::ElfFile& elf) {
  vm::debug::Disassembler d;
  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(elf);
  debugger.StepToTheEnd();
}

void RunSilent(vm::ElfFile& elf) {
  vm::BytecodeInterpreter interpreter;
  interpreter.Load(elf);
  interpreter.RunToTheEnd();
}

int main(int argc, char** argv) {
  if (argc == 1) {
    fmt::print("Please provide a file as the first argument\n");
    exit(0);
  }

  auto path = std::string{argv[1]};

  std::ifstream file(path);

  auto stream =
      std::stringstream{std::string((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>())};

  Parser p{lex::Lexer{stream}};

  std::vector<Statement*> statements;

  try {
    while (true) {
      auto stmt = p.ParseStatement();
      statements.push_back(stmt);
    }
  } catch (parse::errors::ParseError& e) {
    fmt::print(stderr, "Parse error: {}\n", e.what());
  }

  types::check::TypeChecker tchk;

  try {
    for (auto stmt : statements) {
      tchk.Eval(stmt);
    }
  } catch (types::check::TypeError& type_error) {
    fmt::print("Type error: {}\n", type_error.what());
  }

  vm::codegen::Compiler compiler;

  // Compile chunk

  vm::ElfFile elf{{}, {}, {}, {}};
  for (auto stmt : statements) {
    elf += compiler.Compile(stmt);
  }

  if (argc >= 3 && !strcmp(argv[2], "--debug")) {
    RunPhony(elf);
  } else {
    RunSilent(elf);
  }
}
