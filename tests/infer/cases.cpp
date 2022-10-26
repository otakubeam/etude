#include <ast/scope/context_builder.hpp>

#include <types/check/algorithm_w.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

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

  global_context.Print();
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

  global_context.Print();

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  global_context.Print();
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
  global_context.Print();
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:recursive:simple", "[infer]") {
  char stream[] = "fun fib n = { if n == 0 fib(n - 1) else {  1 } };";
  std::stringstream source{stream};
  lex::Lexer l{source};
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

//////////////////////////////////////////////////////////////////////
