#include <ast/scope/context_builder.hpp>
#include <types/check/algorithm_w.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <fmt/color.h>

#include <fstream>
#include <string>

int main(int, char** argv) {
  auto path = std::string{argv[1]};

  std::ifstream file(path);

  auto stream =
      std::stringstream{std::string((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>())};
  lex::Lexer l{stream};
  Parser p{l};
  auto result = p.ParseUnit();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  global_context.Print();

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  global_context.Print();
}
