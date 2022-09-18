#include <types/check/type_checker.hpp>
#include <types/check/type_error.hpp>

#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Checker: Just works", "[checker]") {
  std::stringstream source("1 + true");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Check variable declaration", "[checker]") {
  std::stringstream source(
      "var a = true; "
      "1 + a");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Correct if", "[checker]") {
  std::stringstream source("if true { \"Hello\" } else { \"world\" }");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  CHECK(tchk.Eval(p.ParseExpression()) == &types::builtin_string);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If wrong condition", "[checker]") {
  std::stringstream source("if 1 { \"Hello\" } else { \"world\" }");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If diverging arms", "[checker]") {
  std::stringstream source("if true { 123 } else { \"world\" }");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
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

  types::check::TypeChecker tchk;
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

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
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

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
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

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
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

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseExpression()), types::check::TypeError);
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

  types::check::TypeChecker tchk;
  CHECK_NOTHROW(tchk.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Typechecking fun params", "[checker]") {
  std::stringstream source(  //
      "fun takingFunction(f: (Bool) Int ) Int {"
      "   f(123)                               "
      "}                                       ");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  CHECK_THROWS_AS(tchk.Eval(p.ParseStatement()), types::check::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Typechecking fun arguments", "[checker]") {
  std::stringstream source(  //
      "   # This is ok \n                               "
      "                                                 "
      "   fun takingBool(b: Bool) String {              "
      "      if b { \" True !\" }                       "
      "      else { \" False\" }                        "
      "   }                                             "
      "                                                 "
      "   # This is alright too \n                      "
      "                                                 "
      "   fun takingFunction(f: (Bool) Int ) Int {      "
      "      f(true)                                    "
      "   }                                             "
      "                # But this is clearly wrong \n   "
      "   var result = takingFunction(takingBool);      "
      "                                                 ");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());

  CHECK_THROWS_AS(tchk.Eval(p.ParseStatement()), types::check::TypeError);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Typechecking correct structs", "[checker]") {
  std::stringstream source(  //
      "                                                 "
      "   struct Str {                                  "
      "     a: Int,                                     "
      "     b: Bool,                                    "
      "   };                                            "
      "                                                 "
      "   var inst = Str:{123, true};                   "
      "                                                 "
      "   fun takingStruct(str: Str) Int {              "
      "     var local = str.a;                          "
      "     local                                       "
      "   }                                             "
      "                                                 "
      "   takingStruct(inst);                           "
      "                                                 ");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
  CHECK_NOTHROW(tchk.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Typechecking correct nested", "[checker]") {
  std::stringstream source(  //
      "                                                 "
      "   struct Base {                                 "
      "     a: Int,                                     "
      "   };                                            "
      "                                                 "
      "   struct Nested {                               "
      "     b: Base,                                    "
      "   };                                            "
      "                                                 "
      "   var inst = Nested:{Base:{123}};               "
      "                                                 "
      "   fun takingStruct(str: Nested) Base {          "
      "     str.b                                       "
      "   }                                             "
      "                                                 "
      "   fun takingStruct(str: Nested) Int {           "
      "     str.b.a                                     "
      "   }                                             "
      "                                                 "
      "   takingStruct(inst);                           "
      "                                                 ");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
  CHECK_NOTHROW(tchk.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("checker:initialize-ptrs", "[checker]") {
  std::stringstream source(  //
      "                                                 "
      "   struct Tree {                                 "
      "     left: *Tree,                                "
      "     right: *Tree,                               "
      "   };                                            "
      "                                                 "
      "   var tree = Tree:{unit, unit};                 "
      "                                                 ");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("checker:no-final-expr", "[checker]") {
  std::stringstream source(  //
      "      fun sum(b: Bool) Bool {             "
      "         if b { return true; } else       "
      "              { return false; };          "
      "      }                                   "
      "                                          "
      "      sum(true);                          ");
  Parser p{lex::Lexer{source}};

  types::check::TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseStatement());
}

//////////////////////////////////////////////////////////////////////
