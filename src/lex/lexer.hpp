#pragma once

#include <lex/ident_table.hpp>
#include <lex/lexer_aux.hpp>
#include <lex/token.hpp>

#include <fmt/format.h>

#include <optional>
#include <string>

namespace lex {

class Lexer {
 public:
  Lexer(std::istream& source) : scanner_{source} {
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

  void Advance() {
    prev_ = peek_;

    if (!need_advance) {
      need_advance = true;
    } else {
      peek_ = GetNextToken();
      need_advance = false;
    }
  }

  bool Matches(lex::TokenType type) {
    if (Peek().type != type) {
      return false;
    }

    Advance();
    return true;
  }

  Token Peek() {
    if (need_advance) {
      Advance();
    }
    return peek_;
  }

  Token PreviousToken() {
    return prev_;
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
  // For easy access to locations
  Token prev_{};

  // Current token
  Token peek_{};

  Scanner scanner_;
  bool need_advance = true;

  IdentTable table_;
};

}  // namespace lex
