#include <ast/scope/context_builder.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <fmt/color.h>

#include <fstream>
#include <string>

std::string DumpSymbolTable(ast::scope::ContextBuilder& ctx_builder) {
  std::string dump;
  dump.reserve(4096);
  auto inserter = std::back_inserter(dump);

  fmt::format_to(inserter, "Final symbol table: \n");

  for (auto leaf : ctx_builder.debug_context_leafs_) {
    for (auto& sym : leaf->bindings.symbols) {
      fmt::format_to(inserter, "{}\n", sym.FormatSymbol());
    }
  }

  return dump;
}

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
}
