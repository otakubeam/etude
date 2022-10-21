#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

///////////////////////////////////////////////////////////////////

auto Parser::ParseUnit() -> std::vector<Statement*> {
  auto result = std::vector<Statement*>{};
  while (auto declaration = ParseDeclaration()) {
    result.push_back(declaration);
  }
  Consume(lex::TokenType::TOKEN_EOF);
  return result;
}

///////////////////////////////////////////////////////////////////

Statement* Parser::ParseDeclaration() {
  if (auto struct_declaration = ParseStructDeclStatement()) {
    return struct_declaration;
  }

  if (auto var_declaration = ParseVarDeclStatement()) {
    return var_declaration;
  }

  if (auto fun_declaration = ParseFunDeclStatement()) {
    return fun_declaration;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////

FunDeclStatement* Parser::ParseFunDeclStatement() {
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto function_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto typed_formals = ParseFormals();
  auto return_type = ParseType();

  if (auto block = dynamic_cast<BlockExpression*>(ParseBlockExpression())) {
    return new FunDeclStatement{function_name, return_type,
                                std::move(typed_formals), block};
  }

  throw parse::errors::ParseTrueBlockError{
      function_name.location.Format(),
  };
}

///////////////////////////////////////////////////////////////////

StructDeclStatement* Parser::ParseStructDeclStatement() {
  if (!Matches(lex::TokenType::STRUCT)) {
    return nullptr;
  }

  // 1. Get the name of the new struct

  auto struct_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);
  Consume(lex::TokenType::LEFT_CBRACE);

  // 2. Parse contents

  std::vector<lex::Token> fields;
  std::vector<types::Type*> types;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    fields.push_back(lexer_.GetPreviousToken());
    Consume(lex::TokenType::COLON);

    if (auto type = ParseType()) {
      types.push_back(type);
    } else {
      throw parse::errors::ParseTypeError{FormatLocation()};
    }

    // Demand the trailing comma!
    Consume(lex::TokenType::COMMA);
  }

  Consume(lex::TokenType::RIGHT_CBRACE);
  Consume(lex::TokenType::SEMICOLON);

  return new StructDeclStatement{struct_name, std::move(fields),
                                 std::move(types)};
}

///////////////////////////////////////////////////////////////////

auto Parser::ParseFormals() -> std::vector<FunDeclStatement::FormalParam> {
  Consume(lex::TokenType::LEFT_PAREN);

  std::vector<FunDeclStatement::FormalParam> typed_formals;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    auto param_name = lexer_.GetPreviousToken();
    Consume(lex::TokenType::COLON);

    if (auto type = ParseType()) {
      typed_formals.push_back(FunDeclStatement::FormalParam{
          .ident = param_name,
          .type = type,
      });
    } else {
      throw parse::errors::ParseTypeError{FormatLocation()};
    }

    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }
  }

  Consume(lex::TokenType::RIGHT_PAREN);
  return typed_formals;
}

///////////////////////////////////////////////////////////////////

VarDeclStatement* Parser::ParseVarDeclStatement() {
  if (!Matches(lex::TokenType::VAR)) {
    return nullptr;
  }

  // 1. Get a name to assign to

  Consume(lex::TokenType::IDENTIFIER);
  auto lvalue = new VarAccessExpression{lexer_.GetPreviousToken()};

  // 2. Get an expression to assign to

  Consume(lex::TokenType::ASSIGN);

  auto value = ParseExpression();

  Consume(lex::TokenType::SEMICOLON);

  return new VarDeclStatement{lvalue, value};
}

///////////////////////////////////////////////////////////////////
