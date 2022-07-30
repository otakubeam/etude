#include <ast/visitors/evaluator.hpp>

#include <parse/parse.hpp>

int main() {
  Evaluator e;
  Parser p(lex::Lexer{std::cin});

  while (true) {
    auto stmt = p.ParseStatement();
    std::cout << "[!] Parsed" << std::endl;

    e.Eval(stmt);
    std::cout << "[~] Evaluated" << std::endl;
  }
}
