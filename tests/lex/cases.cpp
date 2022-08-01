#include <lex/lexer.hpp>

// Finally,
#include <catch2/catch.hpp>

#include <iostream>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Lexer: Just works", "[lex]") {
  std::stringstream source("1 + 2");
  lex::Lexer l{source};

  REQUIRE(l.Peek().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::PLUS);
  CHECK(l.Advance().type == lex::TokenType::NUMBER);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Braces", "[lex]") {
  std::stringstream source("1 + (1)");
  lex::Lexer l{source};

  REQUIRE(l.Peek().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::PLUS);
  CHECK(l.Advance().type == lex::TokenType::LEFT_BRACE);
  CHECK(l.Advance().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::RIGHT_BRACE);
}

///////////////////////////////////////////////////////////////////

TEST_CASE("Keywords", "[lex]") {
  std::stringstream source(
      "var fun for if else "
      "return yield true false");
  lex::Lexer l{source};

  REQUIRE(l.Peek().type == lex::TokenType::VAR);
  CHECK(l.Advance().type == lex::TokenType::FUN);
  CHECK(l.Advance().type == lex::TokenType::FOR);
  CHECK(l.Advance().type == lex::TokenType::IF);
  CHECK(l.Advance().type == lex::TokenType::ELSE);
  CHECK(l.Advance().type == lex::TokenType::RETURN);
  CHECK(l.Advance().type == lex::TokenType::YIELD);
  CHECK(l.Advance().type == lex::TokenType::TRUE);
  CHECK(l.Advance().type == lex::TokenType::FALSE);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Consequent", "[lex]") {
  std::stringstream source("!true");
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::NOT);
  CHECK(l.Advance().type == lex::TokenType::TRUE);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Comments", "[lex]") {
  std::stringstream source(
      "# Comment if var a = 1; \n"
      "# One more comment \n"
      "1 # Token then comment \n"  // <--- Token
      "# Comment with no newline");
  lex::Lexer l{source};

  // parses to just `1`
  REQUIRE(l.Peek().type == lex::TokenType::NUMBER);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Statement", "[lex]") {
  std::stringstream source("var abc = 0;");
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::VAR);
  CHECK(l.Advance().type == lex::TokenType::IDENTIFIER);
  CHECK(l.Advance().type == lex::TokenType::ASSIGN);
  CHECK(l.Advance().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::SEMICOLUMN);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("String literal", "[lex]") {
  std::stringstream source("\"Hello world\"");
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::STRING);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Funtion declaration noargs", "[lex]") {
  std::stringstream source("fun f     ()       123;");
  //                        -----  --------  -------------
  //                        name   no args   expr-statement
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::FUN);
  CHECK(l.Advance().type == lex::TokenType::IDENTIFIER);
  CHECK(l.Advance().type == lex::TokenType::LEFT_BRACE);
  CHECK(l.Advance().type == lex::TokenType::RIGHT_BRACE);
  CHECK(l.Advance().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::SEMICOLUMN);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Funtion declaration args", "[lex]") {
  std::stringstream source("fun f     (a1, a2, a3)       123;");
  //                        -----     -------------  -------------
  //                        name          args       expr-statement
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::FUN);
  CHECK(l.Advance().type == lex::TokenType::IDENTIFIER);
  CHECK(l.Advance().type == lex::TokenType::LEFT_BRACE);

  /// Args
  CHECK(l.Advance().type == lex::TokenType::IDENTIFIER);
  CHECK(l.Advance().type == lex::TokenType::COMMA);

  CHECK(l.Advance().type == lex::TokenType::IDENTIFIER);
  CHECK(l.Advance().type == lex::TokenType::COMMA);

  CHECK(l.Advance().type == lex::TokenType::IDENTIFIER);
  ///

  CHECK(l.Advance().type == lex::TokenType::RIGHT_BRACE);
  CHECK(l.Advance().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::SEMICOLUMN);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Curly", "[lex]") {
  std::stringstream source("{ }");
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::LEFT_CBRACE);
  CHECK(l.Advance().type == lex::TokenType::RIGHT_CBRACE);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Assign vs Equals", "[lex]") {
  std::stringstream source("== = ==");
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::EQUALS);
  CHECK(l.Advance().type == lex::TokenType::ASSIGN);
  CHECK(l.Advance().type == lex::TokenType::EQUALS);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Lex types", "[lex]") {
  std::stringstream source("123: int bool string");
  lex::Lexer l{source};

  CHECK(l.Peek().type == lex::TokenType::NUMBER);
  CHECK(l.Advance().type == lex::TokenType::COLUMN);
  CHECK(l.Advance().type == lex::TokenType::TY_INT);
  CHECK(l.Advance().type == lex::TokenType::TY_BOOL);
  CHECK(l.Advance().type == lex::TokenType::TY_STRING);
}

//////////////////////////////////////////////////////////////////////
