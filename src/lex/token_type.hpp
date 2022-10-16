#pragma once

#include <cstdlib>

namespace lex {

//////////////////////////////////////////////////////////////////////

enum class TokenType {
  NUMBER,
  STRING,
  IDENTIFIER,

  UNIT,

  TRUE,
  FALSE,

  PLUS,
  MINUS,

  ASSIGN,
  EQUALS,
  LT,

  LEFT_BRACE,
  RIGHT_BRACE,

  // CURLY
  LEFT_CBRACE,
  RIGHT_CBRACE,

  // SQUARE
  LEFT_SBRACE,
  RIGHT_SBRACE,

  NOT,

  ADDR,
  STAR,
  NEW,

  FUN,
  DOT,
  COMMA,

  VAR,
  TYPE,
  STRUCT,

  TY_INT,
  TY_BOOL,
  TY_UNIT,
  TY_STRING,

  IF,
  ELSE,
  FOR,

  COLUMN,
  SEMICOLUMN,
  RETURN,
  YIELD,

  // These only change the location info of the lexer
  TOKEN_EOL,
  TOKEN_EOF,
};

////////////////////////////////////////////////////////////////

const char* FormatTokenType(TokenType type);

////////////////////////////////////////////////////////////////

}  // namespace lex
