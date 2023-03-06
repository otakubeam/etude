#include <parse/parser.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseKeywordExpresssion() {
  auto tok = lexer_.Peek();

  switch (tok.type) {
    case lex::TokenType::RETURN:
      return ParseReturnExpression();

    case lex::TokenType::YIELD:
      return ParseYieldExpression();

    case lex::TokenType::MATCH:
      return ParseMatchExpression();

    case lex::TokenType::NEW:
      return ParseNewExpression();

    case lex::TokenType::IF:
      return ParseIfExpression();

    default:
      return nullptr;
  }
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseIfExpression() {
  if (!Matches(lex::TokenType::IF)) {
    return nullptr;
  }

  auto location_token = lexer_.GetPreviousToken();

  auto condition = ParseExpression();

  // Optionally consume then keyword
  Matches(lex::TokenType::THEN);

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

Expression* Parser::ParseMatchExpression() {
  if (!Matches(lex::TokenType::MATCH)) {
    return nullptr;
  }

  auto against = ParseExpression();

  Consume(lex::TokenType::LEFT_CBRACE);

  std::vector<MatchExpression::Bind> binds;

  while (Matches(lex::TokenType::BIT_OR)) {
    auto pat = ParsePattern();
    Consume(lex::TokenType::COLON);

    auto expression = ParseExpression();
    binds.push_back({pat, expression});
  }

  Consume(lex::TokenType::RIGHT_CBRACE);

  return new MatchExpression(against, std::move(binds));
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseNewExpression() {
  if (!Matches(lex::TokenType::NEW)) {
    return nullptr;
  }

  auto new_tok = lexer_.GetPreviousToken();

  Expression* size = nullptr;

  if (Matches(lex::TokenType::LEFT_SBRACE)) {
    size = ParseExpression();
    Consume(lex::TokenType::RIGHT_SBRACE);
  }

  auto type = ParseFunctionType();

  Expression* intial_value = nullptr;

  if (Matches(lex::TokenType::LEFT_CBRACE)) {
    intial_value = ParseExpression();
    Consume(lex::TokenType::RIGHT_CBRACE);
  }

  return new NewExpression{new_tok, size, intial_value, type};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseReturnExpression() {
  if (!Matches(lex::TokenType::RETURN)) {
    return nullptr;
  }

  auto return_token = lexer_.GetPreviousToken();
  Expression* ret_expr = ParseExpression();

  return new ReturnExpression{return_token, ret_expr};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseYieldExpression() {
  if (!Matches(lex::TokenType::YIELD)) {
    return nullptr;
  }

  auto location_token = lexer_.GetPreviousToken();
  Expression* yield_value = ParseExpression();

  return new YieldExpression{location_token, yield_value};
}

///////////////////////////////////////////////////////////////////
