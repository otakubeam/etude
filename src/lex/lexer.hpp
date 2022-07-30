#pragma once

#include <lex/ident_table.hpp>
#include <lex/lexer_aux.hpp>
#include <lex/token.hpp>

#include <catch2/catch.hpp>
#include <fmt/format.h>

#include <optional>
#include <string>

namespace lex {

class Lexer {
 public:
  Lexer(std::istream& source) : scanner_{source} {
    Advance();  // so that it starts in a valid state for Peek()
  }

  ////////////////////////////////////////////////////////////////////

  Token GetNextToken() {
    SkipWhitespace();

    SkipComments();

    if (auto op = MatchOperators()) {
      return *op;
    }

    if (auto lit = MatchLiterls()) {
      return *lit;
    }

    if (auto word = MatchWords()) {
      return *word;
    }

    FMT_ASSERT(false, "\nCould not match any token\n");
  }

  ////////////////////////////////////////////////////////////////////

  Token Advance() {
    peek_ = GetNextToken();
    return peek_;
  }

  Token Peek() const {
    return peek_;
  }

 private:
  void SkipWhitespace();

  void SkipComments();

  ////////////////////////////////////////////////////////////////////

  std::optional<Token> MatchOperators();

  std::optional<TokenType> MatchOperator();

  ////////////////////////////////////////////////////////////////////

  std::optional<Token> MatchLiterls();

  std::optional<Token> MatchNumericLiteral();

  std::optional<Token> MatchStringLiteral();

  ////////////////////////////////////////////////////////////////////

  std::optional<Token> MatchWords();

  std::string BufferWord();

  ////////////////////////////////////////////////////////////////////

 private:
  Token peek_{};
  Scanner scanner_;
  IdentTable table_;
};

}  // namespace lex
