#include <lex/lexer.hpp>

#include <cstdlib>

namespace lex {

Lexer::Lexer(std::string_view filename, std::istream& source) : scanner_{filename, source} {
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

    case '!':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::NOT_EQ;
      } else {
        return TokenType::NOT;
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

    case '-':
      if (scanner_.PeekNextSymbol() == '>') {
        scanner_.MoveRight();
        return TokenType::ARROW;
      } else if (scanner_.PeekNextSymbol() == '=') {
        return TokenType::MINUS_EQ;
      } else {
        return TokenType::MINUS;
      }

    case '~':
      if (scanner_.PeekNextSymbol() == '>') {
        scanner_.MoveRight();
        return TokenType::ARROW_CAST;
      } else {
        std::abort();
      }

    case '+':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::PLUS_EQ;
      } else {
        return TokenType::PLUS;
      }

    case '*':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::STAR_EQ;
      } else {
        return TokenType::STAR;
      }

    case '/':
      if (scanner_.PeekNextSymbol() == '=') {
        scanner_.MoveRight();
        return TokenType::DIV_EQ;
      } else {
        return TokenType::DIV;
      }

    case '&':
      return TokenType::ADDR;
    case '|':
      return TokenType::BIT_OR;
    case '(':
      return TokenType::LEFT_PAREN;
    case ')':
      return TokenType::RIGHT_PAREN;
    case '{':
      return TokenType::LEFT_CBRACE;
    case '}':
      return TokenType::RIGHT_CBRACE;
    case '[':
      return TokenType::LEFT_SBRACE;
    case ']':
      return TokenType::RIGHT_SBRACE;
    case ';':
      return TokenType::SEMICOLON;
    case ':':
      return TokenType::COLON;
    case ',':
      return TokenType::COMMA;
    case '@':
      return TokenType::ATTRIBUTE;
    case '.':
      return TokenType::DOT;
    case EOF:
      return TokenType::TOKEN_EOF;
    case '_':
      return TokenType::UNDERSCORE;
    default:
      return std::nullopt;
  }
}

////////////////////////////////////////////////////////////////////

std::optional<Token> Lexer::MatchLiterls() {
  if (auto num_token = MatchNumericLiteral()) {
    return num_token;
  }

  if (auto char_token = MatchCharLiteral()) {
    return char_token;
  }

  if (auto string_token = MatchStringLiteral()) {
    return string_token;
  }

  return std::nullopt;
}

////////////////////////////////////////////////////////////////////

int MatchEscapeSymbol(char sym) {
  auto value = 0;
  switch (sym) {
    case '0':
      value = 0;
      break;
    case 'n':
      value = '\n';
      break;
    case 't':
      value = '\t';
      break;
    default:
      std::abort();
  }
  return value;
}

std::optional<Token> Lexer::MatchCharLiteral() {
  if (scanner_.CurrentSymbol() != '\'') {
    return std::nullopt;
  }

  // Consume staring '
  scanner_.MoveRight();

  char value = 0;

  if (scanner_.CurrentSymbol() == '\\') {
    scanner_.MoveRight();
    value = MatchEscapeSymbol(scanner_.CurrentSymbol());
    scanner_.MoveRight();
  } else {
    value = scanner_.CurrentSymbol();
    scanner_.MoveRight();
  }

  // Consume enclosing '
  scanner_.MoveRight();

  return Token{TokenType::CHAR, scanner_.GetLocation(), {value}};
}

////////////////////////////////////////////////////////////////////

std::size_t Lexer::MunchDigits() {
    std::size_t match_span = 0;

  while (isdigit(scanner_.CurrentSymbol())) {
    scanner_.MoveRight();
    match_span += 1;
  }

  return match_span;
}

std::optional<Token> Lexer::MatchNumericLiteral() {
  auto begin = scanner_.GetBufferPosition();

  auto m1 = MunchDigits();

  if (m1 == 0) {
    return std::nullopt;
  }

  errno = 0;

  if (scanner_.CurrentSymbol() == '.') {
      scanner_.MoveRight();

      MunchDigits();

      double result = strtod(begin, nullptr);

      if (errno) {
          throw std::runtime_error{"Lex: error lexing double"};
      }

      return Token{TokenType::DOUBLE, scanner_.GetLocation(), {result}};

  } else {

      long long result = strtoll(begin, nullptr, 10);

      if (errno) {
          throw std::runtime_error{"Lex: error lexing integer"};
      }

      return Token{TokenType::INTEGER, scanner_.GetLocation(), {result}};
  }
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
