#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

Parser::Parser(lex::Lexer l) : lexer_{l} {
}

bool Parser::Matches(lex::TokenType type) {
  if (lexer_.Peek().type != type) {
    return false;
  }

  lexer_.Advance();
  return true;
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
  return lexer_.PreviousToken().location.Format();
}
