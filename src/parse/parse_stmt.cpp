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

  if (auto block = dynamic_cast<BlockExpression*>(ParseBlockExpression())) {
    return new FunDeclStatement{fun_name, std::move(formals), block};
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

  // Lol
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
