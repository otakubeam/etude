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
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:recursive:simple", "[infer]") {
  char stream[] = "fun fib n = if n == 0 1 else fib(n - 1) + fib(n - 2);";
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
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:poly:id", "[infer]") {
  char stream[] =
      "    fun id x = x;      "
      "                       "
      "    fun main = {       "
      "        id(3);         "
      "        id(true);      "
      "    };                 ";
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
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("infer:poly:swap", "[infer]") {
  char stream[] =
      "    fun swap a b = {       "
      "       var t = *a;         "
      "       *a = *b;            "
      "       *b = t;             "
      "    };                     "
      "                           "
      "    fun main = {           "
      "        var a = 3;         "
      "        var b = 4;         "
      "                           "
      "        swap(&a, &b);      "
      "                           "
      "        var t = true;      "
      "        var f = false;     "
      "                           "
      "        swap(&t, &f);      "
      "    };                     ";
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
