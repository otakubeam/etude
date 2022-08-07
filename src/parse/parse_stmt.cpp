#include <parse/parser.hpp>

///////////////////////////////////////////////////////////////////

FunDeclStatement* Parser::ParseFunDeclStatement() {
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto fun_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);
  Consume(lex::TokenType::LEFT_BRACE);

  auto param_ident = lexer_.Peek();

  std::vector<FunDeclStatement::FormalParam> typed_formals;
  std::vector<types::Type*> just_arg_types;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    Consume(lex::TokenType::COLUMN);

    auto type = ParseType();

    just_arg_types.push_back(type);
    typed_formals.push_back(          //
        FunDeclStatement::FormalParam{//
                                      .ident = param_ident,
                                      .type = type});

    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }

    param_ident = lexer_.Peek();
  }

  Consume(lex::TokenType::RIGHT_BRACE);

  auto ret_type = ParseType();

  types::FnType* declared_type =
      new types::FnType{std::move(just_arg_types), ret_type};

  if (auto block = dynamic_cast<BlockExpression*>(ParseBlockExpression())) {
    return new FunDeclStatement{fun_name, declared_type,
                                std::move(typed_formals), block};
  } else {
    throw "Could not parse block expression";
  }
}

///////////////////////////////////////////////////////////////////

ReturnStatement* Parser::ParseReturnStatement() {
  if (!Matches(lex::TokenType::RETURN)) {
    return nullptr;
  }

  Expression* ret_expr = nullptr;
  if (!Matches(lex::TokenType::SEMICOLUMN)) {
    ret_expr = ParseExpression();
    Consume(lex::TokenType::SEMICOLUMN);
  }

  return new ReturnStatement{ret_expr};
}

///////////////////////////////////////////////////////////////////

YieldStatement* Parser::ParseYieldStatement() {
  if (!Matches(lex::TokenType::YIELD)) {
    return nullptr;
  }

  Expression* ret_expr = nullptr;
  if (!Matches(lex::TokenType::SEMICOLUMN)) {
    ret_expr = ParseExpression();
    Consume(lex::TokenType::SEMICOLUMN);
  }

  return new YieldStatement{ret_expr};
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

  auto field_name = lexer_.Peek();
  std::vector<lex::Token> fields_;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    fields_.push_back(field_name);
    Consume(lex::TokenType::COLUMN);
    // TODO: do not ignore
    AssertParsed(ParseType(), "Could not parse type in struct declaration");
    Consume(lex::TokenType::COMMA);
    field_name = lexer_.Peek();
  }

  Consume(lex::TokenType::RIGHT_CBRACE);
  Consume(lex::TokenType::SEMICOLUMN);

  return new StructDeclStatement{struct_name, std::move(fields_)};
}

///////////////////////////////////////////////////////////////////

VarDeclStatement* Parser::ParseVarDeclStatement() {
  if (!Matches(lex::TokenType::VAR)) {
    return nullptr;
  }

  // 1. Get a name to assign to

  auto token = lexer_.Peek();

  Consume(lex::TokenType::IDENTIFIER);
  auto lvalue = new VarAccessExpression{std::move(token)};

  // 2. Get an expression to assign to

  Consume(lex::TokenType::ASSIGN);

  auto value = ParseExpression();
  AssertParsed(value, "Trying to assign a non-existent value");

  Consume(lex::TokenType::SEMICOLUMN);

  return new VarDeclStatement{lvalue, value};
}

///////////////////////////////////////////////////////////////////

ExprStatement* Parser::ParseExprStatement() {
  auto expr = ParseExpression();

  try {
    Consume(lex::TokenType::SEMICOLUMN);
  } catch (...) {
    // So that the last expression in block can be caught
    //   {
    //     stmt;
    //     stmt;   <<<--- parse all of it as statements
    //     expr           but catch the last one
    //   }
    //
    throw new ExprStatement{expr};
  }

  return new ExprStatement{expr};
}
