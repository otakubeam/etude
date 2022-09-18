#include <parse/parser.hpp>

#include <types/repr/pointer_type.hpp>
#include <types/repr/struct_type.hpp>
#include <types/repr/builtins.hpp>
#include <types/repr/fn_type.hpp>

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseType() {
  types::Type* result = nullptr;
  switch (lexer_.Peek().type) {
    case lex::TokenType::TY_INT:
      result = &types::builtin_int;
      break;

    case lex::TokenType::TY_BOOL:
      result = &types::builtin_bool;
      break;

    case lex::TokenType::TY_STRING:
      result = &types::builtin_string;
      break;

    case lex::TokenType::TY_UNIT:
      result = &types::builtin_unit;
      break;

    case lex::TokenType::STAR:
      lexer_.Advance();
      return new types::PointerType{ParseType()};

    // Syntax: (Int) Unit
    case lex::TokenType::LEFT_BRACE:
      return ParseFunctionType();

    case lex::TokenType::IDENTIFIER:
      return ParseStructType();

    default:
      return nullptr;
  }

  // Advance for simple types
  lexer_.Advance();
  return result;
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseFunctionType() {
  Consume(lex::TokenType::LEFT_BRACE);

  std::vector<types::Type*> args;

  while (auto type = ParseType()) {
    args.push_back(type);

    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }
  }

  Consume(lex::TokenType::RIGHT_BRACE);

  auto return_type = ParseType();

  return new types::FnType{std::move(args), return_type};
}

///////////////////////////////////////////////////////////////////

types::Type* Parser::ParseStructType() {
  auto token = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);
  return new types::StructType{token.GetName()};
}

///////////////////////////////////////////////////////////////////
