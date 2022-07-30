#include <parse/parse.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseComparison() {
  Expression* fst = ParseBinary();

  using lex::TokenType;
  auto token = lexer_.Peek();

  while (Matches(TokenType::LT) || Matches(TokenType::EQUALS)) {
    if (auto snd = ParseBinary()) {
      fst = new ComparisonExpression(fst, token, snd);
    } else {
      throw "Incomplete Comparison Expression";
    }
  }

  return fst;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseBinary() {
  Expression* fst = ParseUnary();

  using lex::TokenType;
  auto token = lexer_.Peek();

  while (Matches(TokenType::PLUS) || Matches(TokenType::MINUS)) {
    if (auto snd = ParseUnary()) {
      fst = new BinaryExpression(fst, token, snd);
    } else {
      throw "Parse error: Incomplete Binary Expression";
    }
  }

  return fst;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseUnary() {
  auto token = lexer_.Peek();

  if (Matches(lex::TokenType::MINUS) || Matches(lex::TokenType::NOT)) {
    if (auto expr = ParseFunApplication()) {
      return new UnaryExpression{token, expr};
    } else {
      throw "Parse error: could not parse primary starting with minus";
    }
  }

  auto expr = ParseFunApplication();
  FMT_ASSERT(expr,
             "\n Parse error: "
             "Could not match Unary Expression \n");

  return expr;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseFunApplication() {
  auto fun_name = lexer_.Peek();

  if (!Matches(lex::TokenType::IDENTIFIER)) {
    return ParsePrimary();
  }

  // At this point we have already consumed the token
  // Now it's our responsibility
  if (!Matches(lex::TokenType::LEFT_BRACE)) {
    return new LiteralExpression{std::move(fun_name)};
  }

  std::vector<Expression*> args;

  // Parse csv

  if (!Matches(lex::TokenType::RIGHT_BRACE)) {
    while (auto expr = ParseExpression()) {
      args.push_back(expr);
      if (!Matches(lex::TokenType::COMMA)) {
        break;
      }
    }

    Consume(lex::TokenType::RIGHT_BRACE);
  }

  return new FnCallExpression{fun_name, std::move(args)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParsePrimary() {
  Expression* result = nullptr;

  // Try parsing grouping first

  if (Matches(lex::TokenType::LEFT_BRACE)) {
    if (auto expr = ParseExpression()) {
      if (Matches(lex::TokenType::RIGHT_BRACE)) {
        return expr;
      }
      throw "Parse error: missing right brace";
    }
    throw "Parse error: missing braced expression";
  }

  // Then all the base cases

  auto token = lexer_.Peek();

  switch (token.type) {
    case lex::TokenType::IDENTIFIER:
    case lex::TokenType::NUMBER:
    case lex::TokenType::STRING:
    case lex::TokenType::FALSE:
    case lex::TokenType::TRUE:
      result = new LiteralExpression{std::move(token)};
      break;

    // case lex::TokenType::IDENTIFIER:
    default:
      throw "Parse error: "
               "Could not match primary expression\n";
      break;
  }

  // Advance for all the base cases and return

  lexer_.Advance();
  return result;
}

////////////////////////////////////////////////////////////////////
