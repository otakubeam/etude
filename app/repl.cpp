#include <ast/visitors/evaluator.hpp>

#include <parse/parser.hpp>

#include <chrono>
#include <thread>

int main() {
  Evaluator e;
  auto p = new Parser(lex::Lexer{std::cin});

  while (true) try {
      auto stmt = p->ParseStatement();
      e.Eval(stmt);
    } catch (ParseError e) {
      fmt::print("{}\n", e.msg);

      fmt::print("[!] Resetting parser \n", e.msg);
      std::this_thread::sleep_for(std::chrono::milliseconds(300));

      delete p;  // Reset parser
      p = new Parser(lex::Lexer{std::cin});
    } catch (...) {
      fmt::print("Unrecognized error\n");
    }
}
