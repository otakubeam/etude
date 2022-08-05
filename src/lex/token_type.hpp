#pragma once

#include <cstdlib>

namespace lex {

//////////////////////////////////////////////////////////////////////

enum class TokenType {
  NUMBER,
  STRING,
  IDENTIFIER,

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

  NOT,

  PRINT,

  FUN,
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

// clang-format off
#define AST_NODE_LIST(code) \
  code(NUMBER)              \
  code(STRING)              \
  code(IDENTIFIER)          \
  code(TRUE)                \
  code(FALSE)               \
  code(PLUS)                \
  code(MINUS)               \
  code(ASSIGN)              \
  code(EQUALS)              \
  code(LT)                  \
  code(LEFT_BRACE)          \
  code(RIGHT_BRACE)         \
  code(LEFT_CBRACE)         \
  code(RIGHT_CBRACE)        \
  code(NOT)                 \
  code(PRINT)               \
  code(FUN)                 \
  code(COMMA)               \
  code(VAR)                 \
  code(TYPE)                \
  code(STRUCT)              \
  code(TY_INT)              \
  code(TY_BOOL)             \
  code(TY_UNIT)             \
  code(TY_STRING)           \
  code(IF)                  \
  code(ELSE)                \
  code(FOR)                 \
  code(COLUMN)              \
  code(SEMICOLUMN)          \
  code(RETURN)              \
  code(YIELD)               \
  code(TOKEN_EOL)           \
  code(TOKEN_EOF)
// clang-format on

////////////////////////////////////////////////////////////////

// https://journal.stuffwithstuff.com/2012/01/24/higher-order-macros-in-c/

#define DEFINE_TYPE_STRING(type) \
  case TokenType::type:          \
    return #type;

// There are just several places where this is used
inline const char* FormatTokenType(TokenType type) {
  switch (type) {
    AST_NODE_LIST(DEFINE_TYPE_STRING)
    default:
      break;
  }
  std::abort();
}

#undef DEFINE_TYPE_STRING

////////////////////////////////////////////////////////////////

}  // namespace lex
