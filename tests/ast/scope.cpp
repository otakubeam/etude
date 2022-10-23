#include <lex/lexer.hpp>

// Finally,
#include <catch2/catch.hpp>

#include <iostream>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Lexer: Just works", "[lex]") {
  std::stringstream source("1 + 2");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::NUMBER));
  CHECK(l.Matches(lex::TokenType::PLUS));
  CHECK(l.Matches(lex::TokenType::NUMBER));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////
