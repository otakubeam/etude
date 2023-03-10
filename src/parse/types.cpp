#include <parse/parser.hpp>

#include <types/type.hpp>

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseType() {
  return ParseFunctionType();
}

///////////////////////////////////////////////////////////////////

auto Parser::ParseFuncParameters()
    -> std::pair<types::Parameter*, types::Type*>  //
{
  auto first = ParsePointerType();

  if (!Matches(lex::TokenType::ARROW)) {
    return {nullptr, first};
  }

  auto [rest_param, final] = ParseFuncParameters();

  auto param = new types::Parameter{
      .ty = first,
      .next = rest_param,
  };

  return {param, final};
}

///////////////////////////////////////////////////////////////////

// Int -> (Int -> Bool) -> *Bool
types::Type* Parser::ParseFunctionType() {
  auto [a, b] = ParseFuncParameters();
  return a ? b : types::MakeFunType(a, b);
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePointerType() {
  if (Matches(lex::TokenType::STAR)) {
    return ParseStructType();
  }

  return types::MakeTypePtr(ParsePointerType());
}

///////////////////////////////////////////////////////////////////

types::Member* Parser::ParseStructMembers() {
  if (!Matches(lex::TokenType::IDENTIFIER)) {
    return nullptr;
  }

  auto field = lexer_.GetPreviousToken().GetName();

  Consume(lex::TokenType::COLON);

  auto type = ParseFunctionType();

  // May or may not be, advance

  Matches(lex::TokenType::COMMA);

  return new types::Member{
      .ty = type,
      .field = field,
      .next = ParseStructMembers(),
  };
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseStructType() {
  if (!Matches(lex::TokenType::STRUCT)) {
    return ParseSumType();
  }

  Consume(lex::TokenType::LEFT_CBRACE);
  auto members = ParseStructMembers();

  Consume(lex::TokenType::RIGHT_CBRACE);
  return types::MakeStructType(members);
}

///////////////////////////////////////////////////////////////////

types::Member* Parser::ParseSumMembers() {
  Consume(lex::TokenType::IDENTIFIER);
  auto field = lexer_.GetPreviousToken().GetName();

  auto type = Matches(lex::TokenType::COLON) ? ParseFunctionType()
                                             : &types::builtin_unit;
  auto next = Matches(lex::TokenType::BIT_OR) ? ParseSumMembers()  //
                                              : nullptr;
  return new types::Member{.ty = type, .field = field, .next = next};
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseSumType() {
  if (!Matches(lex::TokenType::SUM)) {
    return ParsePrimitiveType();
  }

  Consume(lex::TokenType::LEFT_CBRACE);

  // The first `|` is optional

  Matches(lex::TokenType::BIT_OR);

  auto members = ParseSumMembers();

  Consume(lex::TokenType::RIGHT_CBRACE);

  return types::MakeSumType(members);
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseTyApp(lex::Token tok) {
  if (Matches(lex::TokenType::LEFT_PAREN)) {
    return types::MakeTyApp(tok, ParseTypeList());
  } else {
    return types::MakeTyApp(tok, nullptr);
  }
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseTyGrouping() {
  if (!Matches(lex::TokenType::LEFT_PAREN)) {
    return nullptr;
  }

  if (Matches(lex::TokenType::RIGHT_PAREN)) {
    return &types::builtin_unit;
  }

  auto type = ParseFunctionType();
  Consume(lex::TokenType::RIGHT_PAREN);
  return type;
}

///////////////////////////////////////////////////////////////////

types::Parameter* Parser::ParseTypeList() {
  if (Matches(lex::TokenType::RIGHT_PAREN)) {
    return nullptr;
  }

  auto ty = ParseType();
  Matches(lex::TokenType::COMMA);

  return new types::Parameter{
      .ty = ty,
      .next = ParseTypeList(),
  };
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParsePrimitiveType() {
  if (auto group = ParseTyGrouping()) {
    return group;
  }

  auto tok = lexer_.Peek();
  lexer_.Advance();

  switch (tok.type) {
    case lex::TokenType::IDENTIFIER:
      return ParseTyApp(tok);

    case lex::TokenType::UNDERSCORE:
      return types::MakeTypeVar();

    case lex::TokenType::TY_INT:
      return &types::builtin_int;

    case lex::TokenType::TY_BOOL:
      return &types::builtin_bool;

    case lex::TokenType::TY_CHAR:
      return &types::builtin_char;

    case lex::TokenType::TY_UNIT:
      return &types::builtin_unit;

    case lex::TokenType::TY_STRING:
      return types::MakeTypePtr(&types::builtin_char);

    default:
      throw parse::errors::ParseTypeError{FormatLocation()};
  }
}

///////////////////////////////////////////////////////////////////
