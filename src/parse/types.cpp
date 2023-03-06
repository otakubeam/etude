#include <parse/parser.hpp>

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

  return ts.empty() ? first : types::MakeFunType(std::move(ts), first);
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePointerType() {
  if (!Matches(lex::TokenType::STAR)) {
    return ParseStructType();
  }

  return types::MakeTypePtr(ParsePointerType());
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseStructType() {
  if (!Matches(lex::TokenType::STRUCT)) {
    return ParseSumType();
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

  return types::MakeStructType(std::move(fields));
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseSumType() {
  if (!Matches(lex::TokenType::SUM)) {
    return ParsePrimitiveType();
  }

  Consume(lex::TokenType::LEFT_CBRACE);

  // The first `|` is optional
  Matches(lex::TokenType::BIT_OR);

  // 2. Parse struct fields

  std::vector<types::Member> fields;

  do {
    Consume(lex::TokenType::IDENTIFIER);

    fields.push_back(types::Member{
        .field = lexer_.GetPreviousToken().GetName(),
    });

    if (Matches(lex::TokenType::COLON)) {
      if (auto type = ParseFunctionType()) {
        fields.back().ty = type;
      } else {
        throw parse::errors::ParseTypeError{FormatLocation()};
      }
    } else {
      fields.back().ty = &types::builtin_never;
    }

  } while (Matches(lex::TokenType::BIT_OR));

  Consume(lex::TokenType::RIGHT_CBRACE);

  return types::MakeSumType(std::move(fields));
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePrimitiveType() {
  if (Matches(lex::TokenType::LEFT_PAREN)) {
    if (Matches(lex::TokenType::RIGHT_PAREN)) {
      return &types::builtin_unit;
    }

    auto type = ParseFunctionType();
    Consume(lex::TokenType::RIGHT_PAREN);
    return type;
  }

  auto tok = lexer_.Peek();
  lexer_.Advance();

  switch (tok.type) {
    case lex::TokenType::IDENTIFIER: {
      std::vector<types::Type*> types;

      if (Matches(lex::TokenType::LEFT_PAREN)) {
        while (!Matches(lex::TokenType::RIGHT_PAREN)) {
          types.push_back(ParseType());
          Matches(lex::TokenType::COMMA);
        }
      }

      return types::MakeTyApp(tok, std::move(types));
    }

    case lex::TokenType::UNDERSCORE:
      return types::MakeTypeVar();

    case lex::TokenType::TY_INT:
      return &types::builtin_int;

    case lex::TokenType::TY_BOOL:
      return &types::builtin_bool;

    case lex::TokenType::TY_CHAR:
      return &types::builtin_char;

    case lex::TokenType::TY_STRING:
      return types::MakeTypePtr(&types::builtin_char);

    case lex::TokenType::TY_UNIT:
      return &types::builtin_unit;

    default:
      throw parse::errors::ParseTypeError{FormatLocation()};
  }
}

///////////////////////////////////////////////////////////////////
