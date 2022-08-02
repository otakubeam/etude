#include <ast/visitors/type_checker.hpp>

#include <parse/parser.hpp>

#include <types/builtins.hpp>
#include <types/fn_type.hpp>
#include <types/type.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Checker: Just works", "[checker]") {
  std::stringstream source(  //
      "      fun sum(n: Int) Int {               "
      "         1                                "
      "      }                                   "
      "                                          "
      "      sum(4)                              ");
  Parser p{lex::Lexer{source}};

  TypeChecker tchk;
  tchk.Eval(p.ParseStatement());
  tchk.Eval(p.ParseExpression());
}
