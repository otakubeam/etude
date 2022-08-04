#include <ast/visitors/type_checker.hpp>

#include <parse/parser.hpp>

#include <types/builtins.hpp>
#include <types/fn_type.hpp>
#include <types/type.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Checker: Just works", "[checker]") {
  std::stringstream source("1 + true");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check variable declaration", "[checker]") {
  std::stringstream source(
      "var a = true; "
      "1 + a");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Correct if", "[checker]") {
  std::stringstream source("if true { \"Hello\" } else { \"world\" }");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK(tchk.Eval(p.ParseExpression()) == &types::builtin_string);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If wrong condition", "[checker]") {
  std::stringstream source("if 1 { \"Hello\" } else { \"world\" }");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If diverging arms", "[checker]") {
  std::stringstream source("if true { 123 } else { \"world\" }");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check correct function", "[checker]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Int) Int {               "
      "         1                                "
      "      }                                   "
      "                                          "
      "      sum(4)                              "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(p.ParseExpression()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check fn call place", "[checker]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Int) Int {               "
      "         1                                "
      "      }                                   "
      "                                          "
      "      sum(true)                           "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check fn decl return value", "[checker]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Int) Bool {              "
      "         1                                "
      "      }                                   "
      "                                          "
      "      sum(4)                              "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check nested return value", "[checker]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Int) Bool {              "
      "         if n { return 123; } else        "
      "              { return 123; };            "
      "                                          "
      "         true                             "
      "      }                                   "
      "                                          "
      "      sum(4)                              "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check fn decl params", "[checker]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Int) Bool {              "
      "         if n { return true; } else       "
      "              { return false; };          "
      "                                          "
      "         true                             "
      "      }                                   "
      "                                          "
      "      sum(4)                              "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check correct recursive", "[checker]") {
  std::stringstream source(  //
      "     fun mul(a: Int, b: Int) Int {        "
      "       if a == 1 { b } else               "
      "         { b + mul(a-1, b) }              "
      "     }                                    "
      "                                          ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////
