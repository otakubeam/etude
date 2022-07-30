#include <ast/visitors/printing_visitor.hpp>
#include <ast/visitors/evaluator.hpp>

#include <rt/base_object.hpp>

#include <parse/parse.hpp>

#include <lex/lexer.hpp>

#include <fmt/core.h>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Expressions", "[ast]") {
  auto ty = lex::TokenType::NUMBER;
  lex::Location loc_dummy;

  auto minus_tk = lex::Token{lex::TokenType::MINUS, loc_dummy};
  auto plus_tk = lex::Token{lex::TokenType::PLUS, loc_dummy};

  auto tk = lex::Token{ty, loc_dummy, 1},  //
      tk2 = lex::Token{ty, loc_dummy, 2};

  auto a1 = LiteralExpression{tk}, a2 = LiteralExpression{tk2};

  auto un1 = UnaryExpression{minus_tk, &a1};
  auto bin1 = BinaryExpression{&a1, minus_tk, &a2},
       bin2 = BinaryExpression{&un1, plus_tk, &bin1};

  Evaluator e;

  auto result1 = GetPrim<int>(e.Eval(&bin1));
  auto result2 = GetPrim<int>(e.Eval(&bin2));

  REQUIRE(result1 == -1);
  REQUIRE(result2 == -2);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Print tree", "[ast]") {
  auto ty = lex::TokenType::NUMBER;
  lex::Location loc_dummy;

  auto minus_tk = lex::Token{lex::TokenType::MINUS, loc_dummy};
  auto plus_tk = lex::Token{lex::TokenType::PLUS, loc_dummy};

  auto tk = lex::Token{ty, loc_dummy, 1},  //
      tk2 = lex::Token{ty, loc_dummy, 2};

  auto a1 = LiteralExpression{tk}, a2 = LiteralExpression{tk2};

  auto un1 = UnaryExpression{minus_tk, &a1};
  auto bin1 = BinaryExpression{&a1, minus_tk, &a2},
       bin2 = BinaryExpression{&un1, plus_tk, &bin1};

  PrintingVisitor p;

  auto result1 = p.Eval(&bin1);
  auto result2 = p.Eval(&bin2);

  fmt::print("{}\n\n\n{}", result1, result2);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Grouping", "[ast]") {
  char stream[] = "1 - (2 - 3)";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(2));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Associativity", "[parser]") {
  char stream[] = "1 - 2 - 3";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(-4));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Booleans", "[parser]") {
  char stream[] = "!true";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(false));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Variable decalration", "[ast]") {
  char stream[] = "var x = 5;";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_NOTHROW(e.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Keep state", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "x + x";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(10));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Multistate", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "var y = 3;"  //
      "x - y";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(2));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Initialize with variable", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "var y = x;"  //
      "x - y";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(0));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Overwrite", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "var x = 4;"  //
      "x";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(4));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Unknown variable", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "y + x";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK_THROWS(e.Eval(p.ParseExpression()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Eval string literal", "[ast]") {
  char stream[] = " \"abc\"";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim('a'));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Eval fn decl", "[ast]") {
  std::stringstream source("fun f     ()       123;");
  //                        -----  --------  -------------
  //                        name   no args   expr-statement
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_NOTHROW(e.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Eval fn decl args", "[ast]") {
  std::stringstream source("fun f  (a1, a2, a3)       123;");
  //                        -----  ------------  -------------
  //                        name       args      expr-statement
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_NOTHROW(e.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Bad scope access", "[ast]") {
  std::stringstream source(" { var x = 5; } x");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK_THROWS(e.Eval(p.ParseExpression()) == FromPrim(6));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Fn call", "[ast]") {
  std::stringstream source(
      "var a = 3;"
      "fun f() { print(a); }"
      "f();");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Intrinsic print", "[ast]") {
  std::stringstream source("print(4, 3)");
  Parser p{lex::Lexer{source}};

  // Side effect: prints "\n\n4 3 \n\n"

  Evaluator e;
  e.Eval(p.ParseExpression());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Return value", "[ast]") {
  std::stringstream source(      //
      "fun f() { return 123; }"  //
      "f()"                      //
  );
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(123));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Yield as break", "[ast]") {
  std::stringstream source(  //
      "{ yield 5;  print(5); }");
  //              -----------
  //              not executed
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_THROWS_AS(e.Eval(p.ParseStatement()), Evaluator::YieldedValue);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If statement (I)", "[ast]") {
  std::stringstream source(  //
      "if false { print(1); } else { print(0); }");
  //                               -----------
  //                               not executed
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If statement (II)", "[ast]") {
  std::stringstream source(  //
      "fun negate(val) { if val { return false; } else { return true; } }"
      "if negate(true) { print(1); } else { print(0); }");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Doesn't work", "[ast]") {
  Evaluator e;
  Parser p(lex::Lexer{std::cin});

  while (auto stmt = p.ParseStatement()) {
    e.Eval(stmt);
  }
}
