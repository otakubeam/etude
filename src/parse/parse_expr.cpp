#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseExpression() {
  if (auto if_expr = ParseIfExpression()) {
    return if_expr;
  }

  if (auto block_expr = ParseBlockExpression()) {
    return block_expr;
  }

  // Think about precedence wrt to * and &
  // This is probably too low

  if (auto deref_expr = ParseDeref()) {
    return deref_expr;
  }

  if (auto addrof_expr = ParseAddressof()) {
    return addrof_expr;
  }

  if (auto new_expr = ParseNewExpression()) {
    return new_expr;
  }

  return ParseComparison();
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseDeref() {
  auto token = lexer_.Peek();

  if (!Matches(lex::TokenType::STAR)) {
    return nullptr;
  }

  auto ptr_expr = ParseExpression();
  return new DereferenceExpression{token, ptr_expr};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseAddressof() {
  auto token = lexer_.Peek();

  if (!Matches(lex::TokenType::ADDR)) {
    return nullptr;
  }

  auto lvalue_expr = dynamic_cast<LvalueExpression*>(ParseExpression());
  return new AddressofExpression{token, lvalue_expr};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseIfExpression() {
  if (!Matches(lex::TokenType::IF)) {
    return nullptr;
  }

  auto location_token = lexer_.GetPreviousToken();

  auto condition = ParseExpression();
  auto true_branch = ParseBlockExpression();

  if (!condition || !true_branch) {
    throw parse::errors::ParseTrueBlockError{
        location_token.location.Format(),
    };
  }

  Expression* false_branch = nullptr;
  if (Matches(lex::TokenType::ELSE)) {
    false_branch = ParseBlockExpression();
  }

  return new IfExpression(condition, true_branch, false_branch);
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseNewExpression() {
  if (!Matches(lex::TokenType::NEW)) {
    return nullptr;
  }

  auto new_tok = lexer_.GetPreviousToken();

  if (!Matches(lex::TokenType::LEFT_SBRACE)) {
    return new NewExpression{new_tok, nullptr, ParseType()};
  }

  auto size = ParseExpression();
  Consume(lex::TokenType::RIGHT_SBRACE);
  return new NewExpression{new_tok, size, ParseType()};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseBlockExpression() {
  auto location_token = lexer_.Peek();

  if (!Matches(lex::TokenType::LEFT_CBRACE)) {
    return nullptr;
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

  return new BlockExpression{location_token, std::move(stmts), final_expr};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseComparison() {
  Expression* first = ParseBinary();

  auto token = lexer_.Peek();
  if (Matches(lex::TokenType::LT) || Matches(lex::TokenType::EQUALS)) {
    auto second = ParseBinary();
    first = new ComparisonExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseBinary() {
  Expression* first = ParseUnary();

  auto token = lexer_.Peek();
  while (Matches(lex::TokenType::PLUS) || Matches(lex::TokenType::MINUS)) {
    auto second = ParseUnary();
    first = new BinaryExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseUnary() {
  auto token = lexer_.Peek();
  if (Matches(lex::TokenType::MINUS) || Matches(lex::TokenType::NOT)) {
    auto expr = ParsePrimary();
    return new UnaryExpression{token, expr};
  }

  return ParsePrimary();
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseFieldAccess(LvalueExpression* expr) {
  do {
    auto field_name = lexer_.Peek();
    Consume(lex::TokenType::IDENTIFIER);
    expr = new FieldAccessExpression(field_name, expr);
  } while (Matches(lex::TokenType::DOT));

  return expr;
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
  // Consume(lex::TokenType::LEFT_BRACE);

  if (Matches(lex::TokenType::RIGHT_BRACE)) {
    return new FnCallExpression{id, {}};
  }

  auto args = ParseCSV();
  Consume(lex::TokenType::RIGHT_BRACE);

  return new FnCallExpression{id, std::move(args)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseConstructionExpression(lex::Token id) {
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
  // Try parsing grouping first

  if (Matches(lex::TokenType::LEFT_BRACE)) {
    auto expr = ParseExpression();

    Consume(lex::TokenType::RIGHT_BRACE);

    if (Matches(lex::TokenType::DOT)) {
      return ParseFieldAccess(dynamic_cast<LvalueExpression*>(expr));
    }

    return expr;
  }

  // Then all the base cases

  auto token = lexer_.Peek();

  switch (token.type) {
    case lex::TokenType::NUMBER:
    case lex::TokenType::STRING:
    case lex::TokenType::FALSE:
    case lex::TokenType::TRUE:
    case lex::TokenType::UNIT:
      lexer_.Advance();
      return new LiteralExpression{token};

    case lex::TokenType::IDENTIFIER: {
      Consume(lex::TokenType::IDENTIFIER);

      if (Matches(lex::TokenType::COLUMN)) {
        return ParseConstructionExpression(token);
      }

      if (Matches(lex::TokenType::LEFT_BRACE)) {
        return ParseFnCallExpression(token);
      }

      if (Matches(lex::TokenType::DOT)) {
        return ParseFieldAccess(new VarAccessExpression{token});
      }

      return new VarAccessExpression{token};
    }

    default: {
      auto location = token.location.Format();
      throw parse::errors::ParsePrimaryError{location};
    }
  }

  FMT_ASSERT(false, "Unreachable!");
}

////////////////////////////////////////////////////////////////////
