#include <ast/visitors/type_checker.hpp>
#include <ast/visitors/evaluator.hpp>

#include <types/type_error.hpp>

#include <parse/parser.hpp>

#include <chrono>
#include <thread>

int main() {
  Evaluator e;
  TypeChecker tchk;

  auto p = new Parser(lex::Lexer{std::cin});

  while (true) try {
      fmt::print("> ");
      auto stmt = p->ParseStatement();

      tchk.Eval(stmt);
      e.Eval(stmt);

    } catch (ParseError e) {
      fmt::print("Parse error: {}\n", e.msg);
      fmt::print("[!] Resetting parser \n", e.msg);
      std::this_thread::sleep_for(std::chrono::milliseconds(800));
      delete p;  // Reset parser
      p = new Parser(lex::Lexer{std::cin});

    } catch (types::TypeError& type_error) {
      fmt::print("Type error: {}\n", type_error.msg);

    } catch (Evaluator::ReturnedValue rv) {
      fmt::print("Bye! Your rv is {}\n", Format(rv.value));
      return 0;

    } catch (...) {
      fmt::print("Unrecognized error\n");
    }
}
