#include <parse/parser.hpp>

///////////////////////////////////////////////////////////////////

FunDeclStatement* Parser::ParseFunDeclStatement() {
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto fun_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);
  Consume(lex::TokenType::LEFT_BRACE);

  //// TODO: separate into fn GetFormals()
  std::vector<lex::Token> formals;
  auto formal_param = lexer_.Peek();

  while (Matches(lex::TokenType::IDENTIFIER)) {
    formals.push_back(formal_param);
    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }
    formal_param = lexer_.Peek();
  }
  //// separate into fn GetFormals()

  Consume(lex::TokenType::RIGHT_BRACE);

  auto block = ParseBlockStatement();
  return new FunDeclStatement{fun_name, std::move(formals), block};
}

///////////////////////////////////////////////////////////////////

BlockStatement* Parser::ParseBlockStatement() {
  if (!Matches(lex::TokenType::LEFT_CBRACE)) {
    return nullptr;
  }

  std::vector<Statement*> stmts;

  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    auto stmt = ParseStatement();
    stmts.push_back(stmt);
  }

  return new BlockStatement{std::move(stmts)};
}

///////////////////////////////////////////////////////////////////

IfStatement* Parser::ParseIfStatement() {
  if (!Matches(lex::TokenType::IF)) {
    return nullptr;
  }

  // This should be fine even without parentheses, right?
  auto condition = ParseExpression();
  AssertParsed(condition,  //
               "If statement without condition");

  auto true_branch = ParseBlockStatement();
  AssertParsed(true_branch,  //
               "If statement without true branch");

  BlockStatement* false_branch = nullptr;

  if (Matches(lex::TokenType::ELSE)) {
    false_branch = ParseBlockStatement();
    AssertParsed(false_branch,  //
                 "Else clause without an associated statement");
  }

  return new IfStatement(condition, true_branch, false_branch);
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

// Doesn't this sound a bit like `break`?

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
  auto lvalue = new LiteralExpression{std::move(token)};

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
  Consume(lex::TokenType::SEMICOLUMN);

  return new ExprStatement{expr};
}
