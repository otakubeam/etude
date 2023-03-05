#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParsePostfixExpressions() {
  auto expr = ParsePrimary();

  while (true) {
    switch (lexer_.Peek().type) {
      case lex::TokenType::ARROW:
        expr = ParseIndirectFieldAccess(expr);

      case lex::TokenType::LEFT_SBRACE:
        expr = ParseIndexingExpression(expr);

      case lex::TokenType::DOT:
        expr = ParseFieldAccess(expr);

      case lex::TokenType::LEFT_PAREN:
        expr = ParseFnCall(expr);

      case lex::TokenType::ARROW_CAST:
        expr = ParseCast(expr);

      default:
        return expr;
    }
  }
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseFnCall(Expression* expr) {
  Consume(lex::TokenType::LEFT_PAREN);
  auto loc = lexer_.GetPreviousToken();

  if (Matches(lex::TokenType::RIGHT_PAREN)) {
    return new FnCallExpression{loc, expr, {}};
  }

  auto args = ParseCSV();
  Consume(lex::TokenType::RIGHT_PAREN);

  return new FnCallExpression{loc, expr, std::move(args)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseCast(Expression* expr) {
  Consume(lex::TokenType::ARROW_CAST);
  auto flowy_arrow = lexer_.GetPreviousToken();
  auto dest_type = ParsePointerType();
  return new TypecastExpression{expr, flowy_arrow, dest_type};
};

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseIndirectFieldAccess(Expression* expr) {
  auto arrow = lexer_.Peek();
  Consume(lex::TokenType::ARROW);

  auto field_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  return expr = new FieldAccessExpression{
             field_name, new DereferenceExpression{arrow, expr}};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseFieldAccess(Expression* expr) {
  Consume(lex::TokenType::DOT);
  auto field_name = lexer_.Peek();

  Consume(lex::TokenType::IDENTIFIER);
  return new FieldAccessExpression{field_name, expr};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseIndexingExpression(Expression* expr) {
  auto begin_idx = lexer_.Peek();
  Consume(lex::TokenType::LEFT_SBRACE);

  auto index = ParseExpression();
  Consume(lex::TokenType::RIGHT_SBRACE);

  return new IndexExpression{begin_idx, expr, index};
}

////////////////////////////////////////////////////////////////////
