// #include <types/check/type_checker.hpp>
// #include <types/check/type_error.hpp>
//
// #include <vm/codegen/compiler.hpp>
// #include <vm/interpreter.hpp>
//
// #include <parse/parse_error.hpp>
// #include <parse/parser.hpp>
//
// #include <fmt/color.h>
//
// #include <fstream>
// // #include <chrono>
// // #include <thread>
//
// bool print_debug_info = true;
//
int main(int, char**) {
  //   if (argc == 1) {
  //     fmt::print("Please provide the file as the first argument\n");
  //     exit(0);
  //   }
  //
  //   auto path = std::string{argv[1]};
  //
  //   std::ifstream file(path);
  //
  //   auto stream =
  //       std::stringstream{std::string((std::istreambuf_iterator<char>(file)),
  //                                     std::istreambuf_iterator<char>())};
  //
  //   Parser p{lex::Lexer{stream}};
  //
  //   char stage = 0;
  //
  //   fmt::print(
  //       "Specify the stage up to which run the driver:\n"
  //       "p: Parse                                     \n"
  //       "t: Typecheck                                 \n"
  //       "c: Compile                                   \n"
  //       "e: Execute                                   \n");
  //
  //   std::cin >> stage;
  //
  //   std::vector<Statement*> statements;
  //
  //   try {
  //     while (true) {
  //       auto stmt = p.ParseStatement();
  //       statements.push_back(stmt);
  //     }
  //   } catch (parse::errors::ParseError& e) {
  //     fmt::print("Parse error: {}\n", e.what());
  //   }
  //
  //   fmt::print("Parse stage finished!\n");
  //
  //   if (stage == 'p') {
  //     return 0;
  //   }
  //
  //   bool had_errors = false;
  //   types::check::TypeChecker tchk;
  //
  //   try {
  //     for (auto stmt : statements) {
  //       tchk.Eval(stmt);
  //     }
  //   } catch (types::check::TypeError& type_error) {
  //     had_errors = true;
  //     fmt::print("Type error: {}\n", type_error.what());
  //   }
  //
  //   fmt::print("Typecheck stage finished!\n");
  //
  //   if (stage == 't' || had_errors) {
  //     return 0;
  //   }
  //
  //   vm::codegen::Compiler compiler;
  //
  //   // Compile chunk
  //
  //   for (auto stmt : statements) {
  //     auto exe = compiler.Compile(stmt);
  //   }
  //
  //   fmt::print("Compile stage finished!\n");
  //
  //   if (stage == 'c') {
  //     // Maybe dump all code?
  //     return 0;
  //   }
  //
  //   auto chunks = compiler.GetChunks();
  //
  //   for (auto ch : chunks) {
  //     ch.Print();
  //   }
  //
  //   vm::BytecodeInterpreter interpreter{std::move(chunks)};
  //
  //   if (argv[2][0] == 'n') {
  //     print_debug_info = false;
  //   }
  //
  //   interpreter.Interpret();
}
