#include <types/check/type_checker.hpp>
#include <types/check/type_error.hpp>

#include <vm/codegen/compiler.hpp>
#include <vm/interpreter.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <fmt/color.h>

#include <fstream>
#include <chrono>
#include <thread>

bool print_debug_info = true;

int main(int argc, char** argv) {
  if (argc == 1) {
    fmt::print("help\n");
    exit(0);
  }

  auto path = std::string{argv[1]};

  std::ifstream file(path);

  auto stream =
      std::stringstream{std::string((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>())};

  Parser p{lex::Lexer{stream}};

  char stage = 0;

  std::cin >> stage;

  std::vector<Statement*> statements;

  try {
    while (true) {
      auto stmt = p.ParseStatement();
      fmt::print("Good");
      statements.push_back(stmt);
    }
  } catch (parse::errors::ParseError& e) {
    fmt::print("Parse error: {}\n", e.what());
  }

  if (stage == 'p') {
    return 0;
  }

  types::check::TypeChecker tchk;
  try {
    for (auto stmt : statements) {
      tchk.Eval(stmt);
    }
  } catch (types::check::TypeError& type_error) {
    fmt::print("Type error: {}\n", type_error.msg);
  }

  if (stage == 't') {
    return 0;
  }

  vm::codegen::Compiler compiler;

  // Compile chunk

  for (auto stmt : statements) {
    auto exe = compiler.Compile(stmt);
  }

  if (stage == 'c') {
    // Maybe dump all code?
    return 0;
  }

  auto chunks = compiler.GetChunks();
  for (auto ch : chunks) {
    ch.Print();
  }

  vm::BytecodeInterpreter interpreter{std::move(chunks)};

  while (true) {
    fmt::print("How many instructions to execute?\n");

    size_t many = 0;
    std::cin >> many;

    if (many == 0) {
      print_debug_info = !print_debug_info;
    }

    for (size_t i = 0; i < many; i++) {
      if (auto instr = interpreter.NextInstruction()) {
        fmt::print("[^] Instr: {}\n", PrintInstr(*instr));
        interpreter.Interpret(instr);
      } else {
        fmt::print("This is the end\n");
        return (0);
      }
    }
  }
}
