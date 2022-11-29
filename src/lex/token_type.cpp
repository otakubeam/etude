#include <lex/token_type.hpp>

namespace lex {

// https://journal.stuffwithstuff.com/2012/01/24/higher-order-macros-in-c/

////////////////////////////////////////////////////////////////

// clang-format off
#define AST_NODE_LIST(code) \
  code(NUMBER)              \
  code(STRING)              \
  code(IDENTIFIER)          \
  code(UNIT)                \
  code(TRUE)                \
  code(FALSE)               \
  code(PLUS)                \
  code(MINUS)               \
  code(DIV)                 \
  code(PLUS_EQ)             \
  code(MINUS_EQ)            \
  code(STAR_EQ)             \
  code(DIV_EQ)              \
  code(ATTRIBUTE)           \
  code(EXPORT)              \
  code(EXTERN)              \
  code(ASSIGN)              \
  code(EQUALS)              \
  code(LT)                  \
  code(LEFT_PAREN)          \
  code(RIGHT_PAREN)         \
  code(LEFT_CBRACE)         \
  code(RIGHT_CBRACE)        \
  code(LEFT_SBRACE)         \
  code(RIGHT_SBRACE)        \
  code(NOT)                 \
  code(ADDR)                \
  code(STAR)                \
  code(ARROW)               \
  code(ARROW_CAST)          \
  code(NEW)                 \
  code(FUN)                 \
  code(DOT)                 \
  code(COMMA)               \
  code(VAR)                 \
  code(TYPE)                \
  code(STRUCT)              \
  code(UNION)               \
  code(OF)                  \
  code(UNDERSCORE)          \
  code(TY_INT)              \
  code(TY_BOOL)             \
  code(TY_UNIT)             \
  code(TY_STRING)           \
  code(IF)                  \
  code(ELSE)                \
  code(FOR)                 \
  code(COLON)               \
  code(SEMICOLON)           \
  code(RETURN)              \
  code(YIELD)               \
  code(TOKEN_EOF)
// clang-format on

////////////////////////////////////////////////////////////////

#define DEFINE_TYPE_STRING(type) \
  case TokenType::type:          \
    return #type;

////////////////////////////////////////////////////////////////

// There are just several places where this is used
const char* FormatTokenType(TokenType type) {
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
