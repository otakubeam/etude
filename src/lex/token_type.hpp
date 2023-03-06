#pragma once

#include <cstdlib>

namespace lex {

//////////////////////////////////////////////////////////////////////

enum class TokenType {
  INTEGER,
  DOUBLE,
  CHAR,
  STRING,
  IDENTIFIER,

  UNIT,

  TRUE,
  FALSE,

  PLUS,
  MINUS,
  DIV,
  MOD,

  PLUS_EQ,
  MINUS_EQ,
  STAR_EQ,
  DIV_EQ, // /=

  ATTRIBUTE,
  EXPORT,
  EXTERN,

  ASSIGN,
  EQUALS,
  NOT_EQ,
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
  LET,
  TYPE,
  TRAIT,
  STRUCT,
  SUM,
  UNION,
  
  OF,
  FOR,
  IMPL,
  UNDERSCORE,


  TY_INT,
  TY_BOOL,
  TY_CHAR,
  TY_UNIT,
  TY_STRING,

  IF,
  MATCH,
  BIT_OR,
  THEN,
  ELSE,

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
