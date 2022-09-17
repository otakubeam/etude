#include <types/check/type_checker.hpp>

#include <vm/codegen/compiler.hpp>
#include <vm/interpreter.hpp>

#include <parse/parser.hpp>

#include <fmt/color.h>

#include <chrono>
#include <thread>

bool print_debug_info = false;

void Cycle() {
  Parser p{lex::Lexer{std::cin}};

  auto expr = p.ParseExpression();

  types::check::TypeChecker tchk;
  tchk.Eval(expr);

  auto res = vm::codegen::Compiler::CompileScript(expr);

  vm::BytecodeInterpreter::InterpretStandalone(*res);
  // print_debug_info = true;
  // for (auto r : *res) {
  //   r.Print();
  // }
}

int main() {
  // Evaluator e;
  // types::check::TypeChecker tchk;
  //
  // auto p = new Parser(lex::Lexer{std::cin});

  while (true) {
    Cycle();
  }

  // while (true) try {
  //     fmt::print(fmt::emphasis::bold, "> ");
  //     auto stmt = p->ParseStatement();
  //
  //     tchk.Eval(stmt);
  //     e.Eval(stmt);
  //
  //   } catch (ParseError e) {
  //     fmt::print("Parse error: {}\n", e.msg);
  //     fmt::print("[!] Resetting parser \n", e.msg);
  //     std::this_thread::sleep_for(std::chrono::milliseconds(800));
  //     delete p;  // Reset parser
  //     p = new Parser(lex::Lexer{std::cin});
  //
  //   } catch (types::check::TypeError& type_error) {
  //     fmt::print("Type error: {}\n", type_error.msg);
  //
  //   } catch (Evaluator::ReturnedValue rv) {
  //     fmt::print("Bye! Your rv is {}\n", Format(rv.value));
  //     return 0;
  //
  //   } catch (...) {
  //     fmt::print("Unrecognized error\n");
  //   }
}
