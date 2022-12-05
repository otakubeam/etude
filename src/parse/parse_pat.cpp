#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

#include <ast/patterns.hpp>

///////////////////////////////////////////////////////////////////

Pattern* Parser::ParsePattern() {
  if (auto variant_pat = ParseVariantPattern()) {
    return variant_pat;
  }

  if (auto binding_pat = ParseBindingPattern()) {
    return binding_pat;
  }

  if (auto literal_pat = ParseLiteralPattern()) {
    return literal_pat;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////

Pattern* Parser::ParseLiteralPattern() {
  if (lexer_.Peek().type == lex::TokenType::LEFT_PAREN) {
    throw "Unexprected symbol";
  }

  auto lit = ParsePrimary()->as<LiteralExpression>();
  return new LiteralPattern{lit};
}

///////////////////////////////////////////////////////////////////

Pattern* Parser::ParseBindingPattern() {
  if (!Matches(lex::TokenType::IDENTIFIER)) {
    return nullptr;
  }
  return new BindingPattern{lexer_.GetPreviousToken()};
}

///////////////////////////////////////////////////////////////////

Pattern* Parser::ParseVariantPattern() {
  if (!Matches(lex::TokenType::DOT)) {
    return nullptr;
  }

  Consume(lex::TokenType::IDENTIFIER);
  auto ident = lexer_.GetPreviousToken();

  return new VariantPattern{ident,
                            lexer_.Peek().type == lex::TokenType::COLON  //
                                ? nullptr
                                : ParsePattern()};
}

///////////////////////////////////////////////////////////////////
