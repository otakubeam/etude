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
  LE,
  GT,
  GE,

  LEFT_PAREN,
  RIGHT_PAREN,

  // CURLY
  LEFT_CBRACE,
  RIGHT_CBRACE,

  // SQUARE
  LEFT_SBRACE,
  RIGHT_SBRACE,

  NOT,

  ADDR,
  STAR,
  ARROW,
  ARROW_CAST,

  NEW,

  FUN,
  DOT,
  COMMA,

  VAR,
  TYPE,
  STRUCT,
  UNION,
  
  OF,
  UNDERSCORE,


  TY_INT,
  TY_BOOL,
  TY_UNIT,
  TY_STRING,

  IF,
  ELSE,
  FOR,

  COLON,
  SEMICOLON,
  RETURN,
  YIELD,

  TOKEN_EOF,
};

////////////////////////////////////////////////////////////////

const char* FormatTokenType(TokenType type);

////////////////////////////////////////////////////////////////

}  // namespace lex
