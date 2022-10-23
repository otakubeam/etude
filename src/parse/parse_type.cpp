#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

#include <types/type.hpp>

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseType() {
  return ParseFunctionType();
}

///////////////////////////////////////////////////////////////////

// Int -> (Int -> Bool) -> *Bool
types::Type* Parser::ParseFunctionType() {
  auto first = ParsePointerType();

  while (Matches(lex::TokenType::ARROW)) {
    // first = new types::FnType{{first}, ParsePointerType()};
  }

  return first;
}

///////////////////////////////////////////////////////////////////

// struct {}
types::Type* Parser::ParsePointerType() {
  if (Matches(lex::TokenType::STAR)) {
    // return new types::PointerType{ParsePointerType()};
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

  std::vector<types::Member> fields;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    fields.push_back(types::Member{
        .field = lexer_.GetPreviousToken().GetName(),
    });

    Consume(lex::TokenType::COLON);

    if (auto type = ParseFunctionType()) {
      fields.back().ty = type;
    } else {
      throw parse::errors::ParseTypeError{FormatLocation()};
    }

    // May or may not be, advance
    Matches(lex::TokenType::COMMA);
  }

  Consume(lex::TokenType::RIGHT_CBRACE);

  // return new types::StructType{std::move(fields)};
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
    case lex::TokenType::UNDERSCORE:
      return nullptr;
      // Here I probably want to allocate a new type variable
      break;

    case lex::TokenType::TY_INT:
      return &types::builtin_int;
      break;

    case lex::TokenType::TY_BOOL:
      return &types::builtin_bool;
      break;

    case lex::TokenType::TY_STRING:
      // return &types::builtin_string;
      break;

    case lex::TokenType::TY_UNIT:
      return &types::builtin_unit;
      break;

    default:
      throw parse::errors::ParseTypeError{FormatLocation()};
  }
}
