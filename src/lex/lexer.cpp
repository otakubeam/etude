#include <lex/lexer.hpp>

namespace lex {

Lexer::Lexer(std::istream& source) : scanner_{source} {
}

////////////////////////////////////////////////////////////////////

Token Lexer::GetNextToken() {
  SkipWhitespace();

  SkipComments();

  if (auto op = MatchOperators()) {
    return *op;
  }

  if (auto lit = MatchLiterls()) {
    return *lit;
  }

  if (auto word = MatchWords()) {
    return *word;
  }

  FMT_ASSERT(false, "\nCould not match any token\n");
}

////////////////////////////////////////////////////////////////////

Token Lexer::GetPreviousToken() {
  return prev_;
}

////////////////////////////////////////////////////////////////////

void Lexer::Advance() {
  prev_ = peek_;

  if (!need_advance) {
    need_advance = true;
  } else {
    peek_ = GetNextToken();
    need_advance = false;
  }
}

////////////////////////////////////////////////////////////////////

bool Lexer::Matches(lex::TokenType type) {
  if (Peek().type != type) {
    return false;
  }

  Advance();
  return true;
}

////////////////////////////////////////////////////////////////////

Token Lexer::Peek() {
  if (need_advance) {
    Advance();
  }
  return peek_;
}

////////////////////////////////////////////////////////////////////

bool IsWhitespace(char ch) {
  return ch == ' ' || ch == '\n' || ch == '\t';
}

void Lexer::SkipWhitespace() {
  while (IsWhitespace(scanner_.CurrentSymbol())) {
    scanner_.MoveRight();
  }
}

////////////////////////////////////////////////////////////////////

void Lexer::SkipComments() {
  while (scanner_.CurrentSymbol() == '#') {
    scanner_.MoveNextLine();
    SkipWhitespace();
  }
}

////////////////////////////////////////////////////////////////////

std::optional<Token> Lexer::MatchOperators() {
  if (auto type = MatchOperator()) {
    scanner_.MoveRight();
    return Token{*type, scanner_.GetLocation()};
  }

  return std::nullopt;
}

////////////////////////////////////////////////////////////////////

std::optional<TokenType> Lexer::MatchOperator() {
  switch (scanner_.CurrentSymbol()) {
    case '=':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::EQUALS;
      } else {
        return TokenType::ASSIGN;
      }

    case '<':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::LE;
      } else {
        return TokenType::LT;
      }

    case '>':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::GE;
      } else {
        return TokenType::GT;
      }

    case '+':
      return TokenType::PLUS;
    case '-':
      return TokenType::MINUS;
    case '*':
      return TokenType::STAR;
    case '&':
      return TokenType::ADDR;
    case '!':
      return TokenType::NOT;
    case '(':
      return TokenType::LEFT_BRACE;
    case ')':
      return TokenType::RIGHT_BRACE;
    case '{':
      return TokenType::LEFT_CBRACE;
    case '}':
      return TokenType::RIGHT_CBRACE;
    case '[':
      return TokenType::LEFT_SBRACE;
    case ']':
      return TokenType::RIGHT_SBRACE;
    case ';':
      return TokenType::SEMICOLUMN;
    case ':':
      return TokenType::COLUMN;
    case ',':
      return TokenType::COMMA;
    case '.':
      return TokenType::DOT;
    case EOF:
      return TokenType::TOKEN_EOF;
    default:
      return std::nullopt;
  }
}

////////////////////////////////////////////////////////////////////

std::optional<Token> Lexer::MatchLiterls() {
  if (auto num_token = MatchNumericLiteral()) {
    return num_token;
  }

  if (auto string_token = MatchStringLiteral()) {
    return string_token;
  }

  return std::nullopt;
}

////////////////////////////////////////////////////////////////////

std::optional<Token> Lexer::MatchNumericLiteral() {
  int result = 0, match_span = 0;

  while (isdigit(scanner_.CurrentSymbol())) {
    result *= 10;
    result += scanner_.CurrentSymbol() - '0';

    scanner_.MoveRight();
    match_span += 1;
  }

  if (match_span == 0) {
    return std::nullopt;
  }

  return Token{TokenType::NUMBER, scanner_.GetLocation(), {result}};
}

////////////////////////////////////////////////////////////////////

auto NotQuote(char first) -> bool {
  return first != '\"';
}

std::optional<Token> Lexer::MatchStringLiteral() {
  if (NotQuote(scanner_.CurrentSymbol())) {
    return std::nullopt;
  }

  // It matched! Now do match the whole string

  // Consume commencing "
  scanner_.MoveRight();

  auto lit = scanner_.ViewWhile<NotQuote>();

  // Consume enclosing "
  scanner_.MoveRight();

  return Token{TokenType::STRING, scanner_.GetLocation(), {lit}};
}

////////////////////////////////////////////////////////////////////

auto WordPart(char ch) -> bool {
  return isalnum(ch) || ch == '_';
}

std::optional<Token> Lexer::MatchWords() {
  auto word = scanner_.ViewWhile<WordPart>();

  FMT_ASSERT(word.size(), "Not even a word");

  auto type = table_.LookupWord(word);

  if (type == TokenType::IDENTIFIER) {
    return Token{type, scanner_.GetLocation(), {word}};
  }

  // So it must be a keyword with the
  // exact type encoded direcly in `type`
  return Token{type, scanner_.GetLocation()};
}

}  // namespace lex
