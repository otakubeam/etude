#include <ast/scope/context_builder.hpp>

#include <types/check/algorithm_w.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

std::string DumpSymbolTableT(ast::scope::ContextBuilder& ctx_builder) {
  std::string dump;
  dump.reserve(4096);
  auto inserter = std::back_inserter(dump);

  fmt::print("Final symbol table: \n");

  for (auto leaf : ctx_builder.debug_context_leafs_) {
    fmt::print("Leaf\n");
    for (auto& sym : leaf->bindings.symbols) {
      fmt::print("Sym\n");
      fmt::print("{}: {}\n", sym.FormatSymbol(),
                 types::FormatType(*sym.as_varbind.type));
    }

    for (auto& sym : leaf->bindings.symbols) {
      fmt::print("Sym\n");
      fmt::print("{}: {}\n", sym.FormatSymbol(),
                 types::FormatType(*sym.as_varbind.type));
    }
  }

  return dump;
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:simple", "[infer]") {
  char stream[] = "fun f x = { x + 1 };";
  std::stringstream source{stream};
  lex::Lexer l{source};
  Parser p{l};
  auto result = p.ParseUnit();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  fmt::print("Unification complete\n");
  fmt::print("{}", DumpSymbolTableT(ctx_builder));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:pointer", "[infer]") {
  char stream[] = "fun f p = { *p + 1 };";
  std::stringstream source{stream};
  lex::Lexer l{source};
  Parser p{l};
  auto result = p.ParseUnit();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  fmt::print("Unification complete\n");
  fmt::print("{}", DumpSymbolTableT(ctx_builder));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:pointer-II", "[infer]") {
  char stream[] = "fun f p = { *(p + *p) };";
  std::stringstream source{stream};
  lex::Lexer l{source};
  Parser p{l};
  auto result = p.ParseUnit();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  fmt::print("Unification complete\n");
  fmt::print("{}", DumpSymbolTableT(ctx_builder));
}

//////////////////////////////////////////////////////////////////////
