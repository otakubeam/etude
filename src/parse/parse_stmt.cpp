#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

///////////////////////////////////////////////////////////////////

Statement* Parser::ParseStatement() {
  if (auto return_statement = ParseReturnStatement()) {
    return return_statement;
  }

  if (auto yield_statement = ParseYieldStatement()) {
    return yield_statement;
  }

  if (auto expression_statement = ParseExprStatement()) {
    return expression_statement;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////

Expression* SpawnUnitNode(lex::Location location) {
  auto uint_token = lex::Token{
      lex::TokenType::UNIT,
      location,
  };

  return new LiteralExpression{uint_token};
}

ReturnStatement* Parser::ParseReturnStatement() {
  if (!Matches(lex::TokenType::RETURN)) {
    return nullptr;
  }

  auto return_token = lexer_.GetPreviousToken();

  Expression* ret_expr = nullptr;
  if (!Matches(lex::TokenType::SEMICOLON)) {
    ret_expr = ParseExpression();
    Consume(lex::TokenType::SEMICOLON);
  } else {
    ret_expr = SpawnUnitNode(return_token.location);
  }

  return new ReturnStatement{return_token, ret_expr};
}

///////////////////////////////////////////////////////////////////

YieldStatement* Parser::ParseYieldStatement() {
  if (!Matches(lex::TokenType::YIELD)) {
    return nullptr;
  }

  auto location_token = lexer_.GetPreviousToken();

  Expression* yield_value = nullptr;
  if (!Matches(lex::TokenType::SEMICOLON)) {
    yield_value = ParseExpression();
    Consume(lex::TokenType::SEMICOLON);
  }

  return new YieldStatement{location_token, yield_value};
}

///////////////////////////////////////////////////////////////////

Statement* Parser::ParseExprStatement() {
  auto expr = ParseExpression();

  if (Matches(lex::TokenType::ASSIGN)) {
    if (auto target = dynamic_cast<LvalueExpression*>(expr)) {
      return ParseAssignment(target);
    }

    throw parse::errors::ParseNonLvalueError{FormatLocation()};
  }

  try {
    Consume(lex::TokenType::SEMICOLON);
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

///////////////////////////////////////////////////////////////////

AssignmentStatement* Parser::ParseAssignment(LvalueExpression* target) {
  auto assignment_loc = lexer_.GetPreviousToken();
  auto value = ParseExpression();
  Consume(lex::TokenType::SEMICOLON);
  return new AssignmentStatement{assignment_loc, target, value};
}
