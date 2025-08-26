#pragma once

#include <lex/ident_table.hpp>
#include <lex/token.hpp>

#include <fmt/format.h>

#include <optional>
#include <string>

namespace lex {

class Lexer {
 public:
  Lexer(lex::InputFile source);

  Token GetNextToken();

  void Advance();

  Token Peek();

  Token GetPreviousToken();

  bool Matches(lex::TokenType type);

  std::filesystem::path GetFileAbsPath() {
    return scanner_.GetFileAbsPath();
  }

  void SetUnit(Module* unit) {
    scanner_.SetUnit(unit);
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

  std::optional<Token> MatchCharLiteral();

  std::optional<Token> MatchWords();

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
