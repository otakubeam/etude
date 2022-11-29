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

  if (auto new_expr = ParseNewExpression()) {
    return new_expr;
  }

  return ParseComparison();
}

///////////////////////////////////////////////////////////////////

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
  auto lvalue_expr = ParseUnary()->as<LvalueExpression>();

  return new AddressofExpression{token, lvalue_expr};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseIfExpression() {
  if (!Matches(lex::TokenType::IF)) {
    return nullptr;
  }

  auto location_token = lexer_.GetPreviousToken();

  auto condition = ParseExpression();
  auto true_branch = ParseExpression();

  if (!condition || !true_branch) {
    throw parse::errors::ParseTrueBlockError{
        location_token.location.Format(),
    };
  }

  Expression* false_branch = nullptr;
  if (Matches(lex::TokenType::ELSE)) {
    false_branch = ParseExpression();
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
    return new NewExpression{new_tok, nullptr, ParseFunctionType()};
  }

  auto size = ParseExpression();
  Consume(lex::TokenType::RIGHT_SBRACE);
  return new NewExpression{new_tok, size, ParseFunctionType()};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseBlockExpression() {
  if (!Matches(lex::TokenType::LEFT_CBRACE)) {
    return nullptr;
  }

  auto location_token = lexer_.GetPreviousToken();

  if (lexer_.Peek().type == lex::TokenType::DOT) {
    return ParseCompoundInitializer(location_token);
  }

  std::vector<Statement*> stmts;
  Expression* final_expr = nullptr;

  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    try {
      if (auto decl = ParseDeclaration()) {
        stmts.push_back(decl);
      } else if (auto stmt = ParseStatement()) {
        stmts.push_back(stmt);
      }
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

  if (MatchesComparisonSign(token.type)) {
    auto second = ParseBinary();
    first = new ComparisonExpression(first, token, second);
  } else if (Matches(lex::TokenType::EQUALS)) {
    // TODO: move out to separate function
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

///////////////////////////////////////////////////////////////////

// Assume lex::TokenType::ARROW has already been parsed
Expression* Parser::ParseIndirectFieldAccess(Expression* expr) {
  auto star = lexer_.GetPreviousToken();

  Consume(lex::TokenType::IDENTIFIER);

  auto field_name = lexer_.GetPreviousToken();

  expr = new FieldAccessExpression{
      field_name,
      new DereferenceExpression{star, expr},
  };

  // Also check for the start of function call
  if (Matches(lex::TokenType::LEFT_PAREN)) {
    expr = ParseFnCallExpression(expr, field_name);
  }

  return expr;
}

///////////////////////////////////////////////////////////////////

// Assume lex::TokenType::DOT has already been parsed
Expression* Parser::ParseFieldAccess(Expression* expr) {
  Consume(lex::TokenType::IDENTIFIER);

  auto field_name = lexer_.GetPreviousToken();

  expr = new FieldAccessExpression{field_name, expr};

  // Also check for the start of function call
  if (Matches(lex::TokenType::LEFT_PAREN)) {
    expr = ParseFnCallExpression(expr, field_name);
  }

  return expr;
}

////////////////////////////////////////////////////////////////////

// Assume non-empty
std::vector<Expression*> Parser::ParseCSV() {
  std::vector<Expression*> exprs;

  while (auto expr = ParseExpression()) {
    exprs.push_back(expr);
    Matches(lex::TokenType::COMMA);
    if (lexer_.Peek().type == lex::TokenType::RIGHT_PAREN) {
      break;
    }
  }

  return exprs;
}

auto Parser::ParseDesignatedList()
    -> std::vector<CompoundInitializerExpr::Member> {
  std::vector<CompoundInitializerExpr::Member> initializers;

  while (Matches(lex::TokenType::DOT)) {
    auto field = lexer_.Peek();
    Consume(lex::TokenType::IDENTIFIER);

    Consume(lex::TokenType::ASSIGN);

    initializers.push_back({
        .field = field,
        .init = ParseExpression(),
    });

    if (!Matches(lex::TokenType::COMMA)) {
      break;
    }
  }

  return initializers;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseIndexingExpression(Expression* expr) {
  auto loc_token = lexer_.GetPreviousToken();

  auto plus = loc_token;
  plus.type = lex::TokenType::PLUS;

  auto add = ParseExpression();

  Consume(lex::TokenType::RIGHT_SBRACE);

  return new DereferenceExpression{loc_token,
                                   new BinaryExpression{expr, plus, add}};
}
////////////////////////////////////////////////////////////////////

Expression* Parser::ParseFnCallExpression(Expression* expr, lex::Token id) {
  // Consume(lex::TokenType::LEFT_PAREN);

  if (Matches(lex::TokenType::RIGHT_PAREN)) {
    return new FnCallExpression{id, expr, {}};
  }

  auto args = ParseCSV();

  Consume(lex::TokenType::RIGHT_PAREN);

  return new FnCallExpression{id, expr, std::move(args)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseFnCallUnnamed(Expression* expr) {
  // Consume(lex::TokenType::LEFT_PAREN);
  auto loc = lexer_.GetPreviousToken().location;

  if (Matches(lex::TokenType::RIGHT_PAREN)) {
    return new FnCallExpression{loc, expr, {}};
  }

  auto args = ParseCSV();
  Consume(lex::TokenType::RIGHT_PAREN);

  return new FnCallExpression{loc, expr, std::move(args)};
}

////////////////////////////////////////////////////////////////////

// Precedence one
// https://en.cppreference.com/w/c/language/operator_precedence
Expression* Parser::ParsePostfixExpressions() {
  auto ParseCast = [this](Expression* expr) {
    auto flowy_arrow = lexer_.GetPreviousToken();
    auto dest_type = ParsePointerType();
    return new TypecastExpression{expr, flowy_arrow, dest_type};
  };

  auto expr = ParsePrimary();

  while (true) {
    if (Matches(lex::TokenType::DOT)) {
      expr = ParseFieldAccess(expr);
      continue;
    }

    if (Matches(lex::TokenType::ARROW)) {
      expr = ParseIndirectFieldAccess(expr);
      continue;
    }

    if (Matches(lex::TokenType::ARROW_CAST)) {
      expr = ParseCast(expr);
      continue;
    }

    if (Matches(lex::TokenType::LEFT_SBRACE)) {
      expr = ParseIndexingExpression(expr);
      continue;
    }

    if (Matches(lex::TokenType::LEFT_PAREN)) {
      expr = ParseFnCallUnnamed(expr);
      continue;
    }

    break;
  }

  return expr;
}

Expression* Parser::ParsePrimary() {
  auto ParseGrouping = [this]() -> Expression* {
    auto expr = ParseExpression();
    Consume(lex::TokenType::RIGHT_PAREN);
    return expr;
  };

  // Try parsing grouping first

  if (Matches(lex::TokenType::LEFT_PAREN)) {
    return ParseGrouping();
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

      if (Matches(lex::TokenType::LEFT_PAREN)) {
        return ParseFnCallExpression(new VarAccessExpression{token}, token);
      } else {
        return new VarAccessExpression{token};
      }
    }

    default: {
      auto location = token.location.Format();
      throw parse::errors::ParsePrimaryError{location};
    }
  }

  FMT_ASSERT(false, "Unreachable!");
}

////////////////////////////////////////////////////////////////////

// of Str(_) {.field = 3, .bar = true,}
//                or
// var t = {.field = 3, .bar = true,};
// takesStrInt(t);

Expression* Parser::ParseCompoundInitializer(lex::Token curly) {
  // Consume(lex::TokenType::LEFT_CBRACE);

  if (Matches(lex::TokenType::RIGHT_CBRACE)) {
    return new CompoundInitializerExpr{curly, {}};
  }

  auto initializers = ParseDesignatedList();
  Consume(lex::TokenType::RIGHT_CBRACE);

  return new CompoundInitializerExpr{curly, std::move(initializers)};
}

////////////////////////////////////////////////////////////////////
