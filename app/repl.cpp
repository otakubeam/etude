#include <ast/visitors/evaluator.hpp>

#include <parse/parse.hpp>

int main() {
  Evaluator e;
  Parser p(lex::Lexer{std::cin});

  while (auto stmt = p.ParseStatement()) {
    e.Eval(stmt);

    std::cout << std::flush;
  }
}
