#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

#include <types/repr/pointer_type.hpp>
#include <types/repr/struct_type.hpp>
#include <types/repr/builtins.hpp>
#include <types/repr/fn_type.hpp>

///////////////////////////////////////////////////////////////////

// Int -> (Int -> Bool) -> *Bool
types::Type* Parser::ParseFunctionType() {
  auto first = ParsePointerType();

  while (Matches(lex::TokenType::ARROW)) {
    first = new types::FnType{{first}, ParsePointerType()};
  }

  return first;
}

///////////////////////////////////////////////////////////////////

// struct {}
types::Type* Parser::ParsePointerType() {
  if (Matches(lex::TokenType::STAR)) {
    return new types::PointerType{ParsePointerType()};
  }
  return ParseStructType();
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseStructType() {
  if (!Matches(lex::TokenType::STRUCT)) {
    return ParsePrimitiveType();
  }

  Consume(lex::TokenType::LEFT_CBRACE);

  // 2. Parse struct fields

  std::vector<types::StructType::Member> fields;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    fields.push_back(types::StructType::Member{
        .name = lexer_.GetPreviousToken().GetName(),
    });

    Consume(lex::TokenType::COLON);

    if (auto type = ParseFunctionType()) {
      fields.back().type = type;
    } else {
      throw parse::errors::ParseTypeError{FormatLocation()};
    }

    // May or may not be, advance
    Matches(lex::TokenType::COMMA);
  }

  Consume(lex::TokenType::RIGHT_CBRACE);

  return new types::StructType{std::move(fields)};
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePrimitiveType() {
  if (Matches(lex::TokenType::LEFT_PAREN)) {
    auto type = ParseFunctionType();
    Consume(lex::TokenType::RIGHT_PAREN);
    return type;
  }

  lexer_.Advance();
  switch (lexer_.GetPreviousToken().type) {
    case lex::TokenType::TY_INT:
      return &types::builtin_int;
      break;

    case lex::TokenType::TY_BOOL:
      return &types::builtin_bool;
      break;

    case lex::TokenType::TY_STRING:
      return &types::builtin_string;
      break;

    case lex::TokenType::TY_UNIT:
      return &types::builtin_unit;
      break;

    default:
      throw parse::errors::ParseTypeError{FormatLocation()};
  }
}
