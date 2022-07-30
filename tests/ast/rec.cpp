#include <ast/visitors/printing_visitor.hpp>
#include <ast/visitors/evaluator.hpp>

#include <rt/base_object.hpp>

#include <parse/parse.hpp>

#include <lex/lexer.hpp>

#include <fmt/core.h>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Recursive", "[ast]") {
  std::stringstream source(  //
      "                                          "
      "      fun sum(n) {                        "
      "         if n == 0 {                      "
      "             return 1;                    "
      "         } else {                         "
      "             return (n + sum(n-1));       "
      "         }                                "
      "      }                                   "
      "                                          "
      "             sum(4)                       ");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(11));
}

//////////////////////////////////////////////////////////////////////
