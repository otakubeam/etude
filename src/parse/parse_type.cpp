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
  std::vector<types::Type*> ts;

  while (Matches(lex::TokenType::ARROW)) {
    ts.push_back(std::move(first));
    first = ParsePointerType();
  }

  return ts.empty() ? first
                    : new types::Type{
                          .tag = types::TypeTag::TY_FUN,
                          .as_fun = {.param_pack = std::move(ts),
                                     .result_type = first},
                      };
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePointerType() {
  if (!Matches(lex::TokenType::STAR)) {
    return ParseStructType();
  }

  return new types::Type{
      .tag = types::TypeTag::TY_PTR,
      .as_ptr = {.underlying = ParsePointerType()},
  };
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

  return new types::Type{.tag = types::TypeTag::TY_STRUCT,
                         .as_struct = {std::move(fields)}};
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePrimitiveType() {
  if (Matches(lex::TokenType::LEFT_PAREN)) {
    auto type = ParseFunctionType();
    Consume(lex::TokenType::RIGHT_PAREN);
    return type;
  }

  auto tok = lexer_.Peek();
  lexer_.Advance();
  switch (tok.type) {
    case lex::TokenType::UNDERSCORE:
      // Here I probably want to allocate a new type variable
      return nullptr;
      break;

    case lex::TokenType::TY_INT:
      return &types::builtin_int;
      break;

    case lex::TokenType::TY_BOOL:
      return &types::builtin_bool;
      break;

    case lex::TokenType::TY_STRING:
      std::abort();
      break;

    case lex::TokenType::TY_UNIT:
      return &types::builtin_unit;
      break;

    default:
      return new types::Type{.tag = types::TypeTag::TY_ALIAS,
                             .as_alias = {.name = tok.GetName()}};
      // throw parse::errors::ParseTypeError{FormatLocation()};
  }
}
