#include <parse/parser.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseExpression() {
  return ParseComparison();
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseComparison() {
  Expression* fst = ParseBinary();

  auto token = lexer_.Peek();
  if (Matches(lex::TokenType::LT) ||  //
      Matches(lex::TokenType::EQUALS)) {
    auto snd = ParseBinary();
    fst = new ComparisonExpression(fst, token, snd);
  }

  return fst;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseBinary() {
  Expression* fst = ParseUnary();

  auto token = lexer_.Peek();
  while (Matches(lex::TokenType::PLUS) ||  //
         Matches(lex::TokenType::MINUS)) {
    auto snd = ParseUnary();
    fst = new BinaryExpression(fst, token, snd);
  }

  return fst;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseUnary() {
  auto token = lexer_.Peek();
  if (Matches(lex::TokenType::MINUS) ||  //
      Matches(lex::TokenType::NOT)) {
    auto expr = ParseIfExpression();
    return new UnaryExpression{token, expr};
  }

  return ParseIfExpression();
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseIfExpression() {
  if (!Matches(lex::TokenType::IF)) {
    return ParseBlockExpression();
  }

  auto condition = ParseExpression();
  auto true_branch = ParseBlockExpression();

  Expression* false_branch = nullptr;
  if (Matches(lex::TokenType::ELSE)) {

    false_branch = ParseBlockExpression();
  }

  return new IfExpression(condition, true_branch, false_branch);
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseBlockExpression() {
  if (!Matches(lex::TokenType::LEFT_CBRACE)) {
    return SwitchOnId();
  }

  std::vector<Statement*> stmts;
  Expression* final_expr = nullptr;

  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    try {
      auto stmt = ParseStatement();
      stmts.push_back(stmt);
    } catch (ExprStatement* e) {
      final_expr = e->expr_;
      Consume(lex::TokenType::RIGHT_CBRACE);
      break;
    }
  }

  return new BlockExpression{std::move(stmts), final_expr};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::SwitchOnId() {
  auto id = lexer_.Peek();

  if (!Matches(lex::TokenType::IDENTIFIER)) {
    return ParsePrimary();
  }

  // Four possibilities:
  // 1. Function call   ~ id '('
  // 2. Struct access   ~ id '.'
  // 3. Struct creation ~ id ':'
  // 4. Variable access ~ id

  switch (lexer_.Peek().type) {
    case lex::TokenType::LEFT_BRACE:
      return ParseFnCallExpression(id);

    case lex::TokenType::COLUMN:
      return ParseConstructionExpression(id);

    case lex::TokenType::DOT: {
      Consume(lex::TokenType::DOT);
      auto field_name = lexer_.Peek();
      Consume(lex::TokenType::IDENTIFIER);

      return new FieldAccessExpression(id, field_name);
    }

    default:
      return new VarAccessExpression(id);
  }
}

////////////////////////////////////////////////////////////////////

// Assume non-empty
std::vector<Expression*> Parser::ParseCSV() {
  std::vector<Expression*> exprs;

  while (auto expr = ParseExpression()) {
    exprs.push_back(expr);
    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }
  }

  return exprs;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseFnCallExpression(lex::Token id) {
  Consume(lex::TokenType::LEFT_BRACE);

  if (Matches(lex::TokenType::RIGHT_BRACE)) {
    return new FnCallExpression{id, {}};
  }

  auto args = ParseCSV();
  Consume(lex::TokenType::RIGHT_BRACE);

  return new FnCallExpression{id, std::move(args)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseConstructionExpression(lex::Token id) {
  Consume(lex::TokenType::COLUMN);
  Consume(lex::TokenType::LEFT_CBRACE);

  if (Matches(lex::TokenType::RIGHT_CBRACE)) {
    return new StructConstructionExpression{id, {}};
  }

  auto initializers = ParseCSV();
  Consume(lex::TokenType::RIGHT_CBRACE);

  return new StructConstructionExpression{id, std::move(initializers)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParsePrimary() {
  Expression* result = nullptr;

  // Try parsing grouping first

  if (Matches(lex::TokenType::LEFT_BRACE)) {
    auto expr = ParseExpression();
    Consume(lex::TokenType::RIGHT_BRACE);
    return expr;
  }

  // Then all the base cases

  auto token = lexer_.Peek();

  switch (token.type) {
    case lex::TokenType::NUMBER:
    case lex::TokenType::STRING:
    case lex::TokenType::FALSE:
    case lex::TokenType::TRUE:
      result = new LiteralExpression{std::move(token)};
      break;

    case lex::TokenType::IDENTIFIER:
      result = new VarAccessExpression{std::move(token)};
      break;

    default:
      throw ParseError{"Could not match primary expression\n"};
  }

  // Advance for all the base cases and return

  lexer_.Advance();
  return result;
}

////////////////////////////////////////////////////////////////////
