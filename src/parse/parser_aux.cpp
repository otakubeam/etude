#include <parse/parser.hpp>

Parser::Parser(lex::Lexer& l) : lexer_{l} {
}

bool Parser::Matches(lex::TokenType type) {
  if (lexer_.Peek().type != type) {
    return false;
  }

  lexer_.Advance();
  return true;
}

bool Parser::MatchesAssignmentSign(lex::TokenType type) {
  switch (type) {
    case lex::TokenType::DIV_EQ:
    case lex::TokenType::ASSIGN:
    case lex::TokenType::STAR_EQ:
    case lex::TokenType::PLUS_EQ:
    case lex::TokenType::MINUS_EQ:
      lexer_.Advance();
      return true;

    default:
      return false;
  }
}

bool Parser::MatchesComparisonSign(lex::TokenType type) {
  switch (type) {
    case lex::TokenType::GE:
    case lex::TokenType::GT:
    case lex::TokenType::LE:
    case lex::TokenType::LT:
    case lex::TokenType::EQUALS:
    case lex::TokenType::NOT_EQ:
      lexer_.Advance();
      return true;

    default:
      return false;
  }
}

void Parser::Consume(lex::TokenType type) {
  if (!Matches(type)) {
    throw parse::errors::ParseTokenError{
        lex::FormatTokenType(type),
        FormatLocation(),
    };
  }
}

// Checks whether `.none` is a tag-only value
// as opposed to `.some <expr>`
bool Parser::TagOnly() {
  auto next_token = lexer_.Peek().type;
  switch (next_token) {
    case lex::TokenType::RIGHT_PAREN:
    case lex::TokenType::RIGHT_CBRACE:
    case lex::TokenType::RIGHT_SBRACE:
    case lex::TokenType::COMMA:
    case lex::TokenType::COLON:
    case lex::TokenType::BIT_OR:
    case lex::TokenType::ELSE:
    case lex::TokenType::SEMICOLON:
    case lex::TokenType::VAR:
      return true;

    default:
      return false;
  }
}

std::string Parser::FormatLocation() {
  return lexer_.GetPreviousToken().location.Format();
}

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
