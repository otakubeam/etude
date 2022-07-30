#include <ast/visitors/evaluator.hpp>

#include <parse/parse.hpp>

int main() {
  Evaluator e;
  Parser p(lex::Lexer{std::cin});

  while (!false) {
    auto stmt = p.ParseStatement();
    e.Eval(stmt);
  }
}
