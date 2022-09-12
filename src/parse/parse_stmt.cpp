#include <parse/parser.hpp>

///////////////////////////////////////////////////////////////////

Statement* Parser::ParseStatement() {
  if (auto strcuct_decl = ParseStructDeclStatement()) {
    return strcuct_decl;
  }

  if (auto var_decl = ParseVarDeclStatement()) {
    return var_decl;
  }

  if (auto fun_decl = ParseFunDeclStatement()) {
    return fun_decl;
  }

  if (auto ret_stmt = ParseReturnStatement()) {
    return ret_stmt;
  }

  if (auto yield_stmt = ParseYieldStatement()) {
    return yield_stmt;
  }

  if (auto expr_stmt = ParseExprStatement()) {
    return expr_stmt;
  }

  std::abort();
}

///////////////////////////////////////////////////////////////////

FunDeclStatement* Parser::ParseFunDeclStatement() {
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto fun_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto typed_formals = ParseFormals();
  auto ret_type = ParseType();

  if (auto block = dynamic_cast<BlockExpression*>(ParseBlockExpression())) {
    return new FunDeclStatement{fun_name, ret_type, std::move(typed_formals),
                                block};
  }

  throw "Could not parse block expression";
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
  std::vector<lex::Token> fields;
  std::vector<types::Type*> types;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    fields.push_back(field_name);
    Consume(lex::TokenType::COLUMN);

    auto t = ParseType();
    AssertParsed(t, "Could not parse type in struct declaration");
    types.push_back(t);

    // Subtly different from Parse Formals
    Consume(lex::TokenType::COMMA);
    field_name = lexer_.Peek();
  }

  Consume(lex::TokenType::RIGHT_CBRACE);
  Consume(lex::TokenType::SEMICOLUMN);

  return new StructDeclStatement{struct_name, std::move(fields),
                                 std::move(types)};
}

///////////////////////////////////////////////////////////////////

auto Parser::ParseFormals() -> std::vector<FunDeclStatement::FormalParam> {
  Consume(lex::TokenType::LEFT_BRACE);

  auto param_ident = lexer_.Peek();

  std::vector<FunDeclStatement::FormalParam> typed_formals;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    Consume(lex::TokenType::COLUMN);

    auto type = ParseType();

    auto fp = FunDeclStatement::FormalParam{.ident = param_ident,  //
                                            .type = type};
    typed_formals.push_back(fp);

    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }

    param_ident = lexer_.Peek();
  }

  Consume(lex::TokenType::RIGHT_BRACE);
  return typed_formals;
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

Statement* Parser::ParseExprStatement() {
  auto expr = ParseExpression();

  if (Matches(lex::TokenType::ASSIGN)) {
    auto target = dynamic_cast<LvalueExpression*>(expr);
    if (!target) {
      throw "Assigning to non-lvalue";
    }
    return ParseAssignment(target);
  }

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

AssignmentStatement* Parser::ParseAssignment(LvalueExpression* target) {
  auto value = ParseExpression();
  Consume(lex::TokenType::SEMICOLUMN);
  return new AssignmentStatement{target, value};
}
