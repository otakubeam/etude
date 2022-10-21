#include <parse/parse_error.hpp>
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

bool Parser::MatchesComparisonSign(lex::TokenType type) {
  switch (type) {
    case lex::TokenType::GE:
    case lex::TokenType::GT:
    case lex::TokenType::LE:
    case lex::TokenType::LT:
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

std::string Parser::FormatLocation() {
  return lexer_.GetPreviousToken().location.Format();
}
