#include <parse/parser.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseSeqExpression() {
  if (auto let = ParseLetExpression()) {
    return let;  // Let is also a kind of `SeqExpression`
  }

  Expression* first = ParseAssignment();

  auto token = lexer_.Peek();

  if (Matches(lex::TokenType::SEMICOLON)) {
    auto second = ParseSeqExpression();
    return new SeqExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseLetExpression() {
  if (!Matches(lex::TokenType::LET)) {
    return nullptr;
  }

  auto let = lexer_.GetPreviousToken();

  auto pat = ParsePattern();

  Consume(lex::TokenType::ASSIGN);

  auto value = ParseAssignment();

  Consume(lex::TokenType::ELSE);

  auto else_rest = ParseAssignment();

  Consume(lex::TokenType::SEMICOLON);

  auto rest = ParseSeqExpression();

  return new LetExpression(let, pat, value, else_rest, rest);
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseAssignment() {
  Expression* target = ParseComparison();

  auto token = lexer_.Peek();

  if (MatchesAssignmentSign(token.type)) {
    auto value = ParseComparison();
    return new AssignExpression(token, target, value);
  }

  return target;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseComparison() {
  Expression* first = ParseAdditive();

  auto token = lexer_.Peek();

  if (MatchesComparisonSign(token.type)) {
    auto second = ParseAdditive();
    return new ComparisonExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseAdditive() {
  Expression* first = ParseMultiplicative();

  while (Matches(lex::TokenType::PLUS) || Matches(lex::TokenType::MINUS)) {
    auto token = lexer_.GetPreviousToken();
    auto second = ParseMultiplicative();
    first = new BinaryExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseMultiplicative() {
  Expression* first = ParseUnary();

  while (Matches(lex::TokenType::STAR) || Matches(lex::TokenType::DIV) ||
         Matches(lex::TokenType::MOD)) {
    auto token = lexer_.GetPreviousToken();
    auto second = ParseUnary();
    first = new BinaryExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseUnary() {
  auto token = lexer_.Peek();

  if (auto deref_expr = ParseDeref()) {
    return deref_expr;
  }

  if (auto addrof_expr = ParseAddressof()) {
    return addrof_expr;
  }

  if (Matches(lex::TokenType::MINUS) || Matches(lex::TokenType::NOT)) {
    auto expr = ParseUnary();
    return new UnaryExpression{token, expr};
  }

  return ParsePostfixExpressions();
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseDeref() {
  if (!Matches(lex::TokenType::STAR)) {
    return nullptr;
  }

  auto token = lexer_.GetPreviousToken();
  auto ptr_expr = ParseUnary();

  return new DereferenceExpression{token, ptr_expr};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseAddressof() {
  if (!Matches(lex::TokenType::ADDR)) {
    return nullptr;
  }

  auto token = lexer_.GetPreviousToken();
  auto lvalue_expr = ParseUnary();

  return new AddressofExpression{token, lvalue_expr};
}

///////////////////////////////////////////////////////////////////
