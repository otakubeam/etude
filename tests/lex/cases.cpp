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

TEST_CASE("Braces", "[lex]") {
  std::stringstream source("1 + (1)");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::NUMBER));
  CHECK(l.Matches(lex::TokenType::PLUS));
  CHECK(l.Matches(lex::TokenType::LEFT_PAREN));
  CHECK(l.Matches(lex::TokenType::NUMBER));
  CHECK(l.Matches(lex::TokenType::RIGHT_PAREN));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

///////////////////////////////////////////////////////////////////

TEST_CASE("Keywords", "[lex]") {
  std::stringstream source(
      "var \nfun \nfor\n if\n else "
      "return yield true false");
  lex::Lexer l{source};
  CHECK(l.Matches(lex::TokenType::VAR));
  CHECK(l.Matches(lex::TokenType::FUN));
  CHECK(l.Matches(lex::TokenType::FOR));
  CHECK(l.Matches(lex::TokenType::IF));
  CHECK(l.Matches(lex::TokenType::ELSE));
  CHECK(l.Matches(lex::TokenType::RETURN));
  CHECK(l.Matches(lex::TokenType::YIELD));
  CHECK(l.Matches(lex::TokenType::TRUE));
  CHECK(l.Matches(lex::TokenType::FALSE));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Consequent", "[lex]") {
  std::stringstream source("!true");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::NOT));
  CHECK(l.Matches(lex::TokenType::TRUE));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Comments", "[lex]") {
  std::stringstream source(
      "# Comment if var a = 1; \n"
      "# One more comment      \n"
      "1 # Token then comment  \n"  // <--- Token
      "# Comment with no newline\n");
  lex::Lexer l{source};

  // parses to just `1`
  CHECK(l.Matches(lex::TokenType::NUMBER));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Statement", "[lex]") {
  std::stringstream source("var abc = 0;");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::VAR));
  CHECK(l.Matches(lex::TokenType::IDENTIFIER));
  CHECK(l.Matches(lex::TokenType::ASSIGN));
  CHECK(l.Matches(lex::TokenType::NUMBER));
  CHECK(l.Matches(lex::TokenType::SEMICOLON));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Bit operations", "[lex]") {
  std::stringstream source("~ ^ & |");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::TILDE));
  CHECK(l.Matches(lex::TokenType::BIT_XOR));
  CHECK(l.Matches(lex::TokenType::ADDR));
  CHECK(l.Matches(lex::TokenType::BIT_OR));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("String literal", "[lex]") {
  std::stringstream source("\"Hello world\"");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::STRING));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Empty string literal", "[lex]") {
  std::stringstream source("\"\"");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::STRING));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Funtion declaration args", "[lex]") {
  std::stringstream source("(a1, a2)");
  //                        -----     -------------  -------------
  //                        name          args       expr-statement
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::LEFT_PAREN));
  CHECK(l.Matches(lex::TokenType::IDENTIFIER));
  CHECK(l.Matches(lex::TokenType::COMMA));
  CHECK(l.Matches(lex::TokenType::IDENTIFIER));
  CHECK(l.Matches(lex::TokenType::RIGHT_PAREN));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Curly", "[lex]") {
  std::stringstream source("{ }");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::LEFT_CBRACE));
  CHECK(l.Matches(lex::TokenType::RIGHT_CBRACE));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Assign vs Equals", "[lex]") {
  std::stringstream source("== = == <= >= > <");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::EQUALS));
  CHECK(l.Matches(lex::TokenType::ASSIGN));
  CHECK(l.Matches(lex::TokenType::EQUALS));
  CHECK(l.Matches(lex::TokenType::LE));
  CHECK(l.Matches(lex::TokenType::GE));
  CHECK(l.Matches(lex::TokenType::GT));
  CHECK(l.Matches(lex::TokenType::LT));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Lex types", "[lex]") {
  std::stringstream source(": Int Bool String Unit:;");
  lex::Lexer l{source};

  CHECK(l.Matches(lex::TokenType::COLON));

  CHECK(l.Matches(lex::TokenType::TY_INT));
  CHECK(l.Matches(lex::TokenType::TY_BOOL));
  CHECK(l.Matches(lex::TokenType::TY_STRING));
  CHECK(l.Matches(lex::TokenType::TY_UNIT));
  CHECK(l.Matches(lex::TokenType::COLON));
  CHECK(l.Matches(lex::TokenType::SEMICOLON));
  CHECK(l.Matches(lex::TokenType::TOKEN_EOF));
}

//////////////////////////////////////////////////////////////////////
