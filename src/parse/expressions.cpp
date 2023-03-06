#include <parse/parser.hpp>

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseExpression() {
  return ParseAssignment();
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseBlockExpression() {
  if (!Matches(lex::TokenType::LEFT_CBRACE)) {
    return nullptr;
  }

  auto curly_brace = lexer_.GetPreviousToken();

  // Check Structure Initialization

  if (lexer_.Peek().type == lex::TokenType::DOT) {
    return ParseCompoundInitializer(curly_brace);
  }

  //
  // Descend into the Sequencing Expression
  //
  //     seq_expr ::= <...> ; <...>
  //

  auto seq = ParseSeqExpression();

  // TODO: also look for delarations!

  Consume(lex::TokenType::RIGHT_CBRACE);

  return new BlockExpression{curly_brace, seq};
}

///////////////////////////////////////////////////////////////////

Expression* Parser::ParseCompoundInitializer(lex::Token curly) {
  Consume(lex::TokenType::LEFT_CBRACE);

  if (Matches(lex::TokenType::RIGHT_CBRACE)) {
    return new CompoundInitializerExpr{curly, {}};
  }

  auto initializers = ParseDesignatedList();
  Consume(lex::TokenType::RIGHT_CBRACE);

  return new CompoundInitializerExpr{curly, std::move(initializers)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseSeqExpression() {
  if (auto let = ParseLetExpression()) {
    return let;  // Let is also a kind of `SeqExpression`
  }

  Expression* first = ParseAssignment();

  auto token = lexer_.Peek();

  if (Matches(lex::TokenType::SEMICOLON)) {
    auto second = ParseSeqExpression();
    return new SeqExpression(first, token, second);
  }

  return first;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseLetExpression() {
  if (!Matches(lex::TokenType::LET)) {
    return nullptr;
  }

  auto let = lexer_.GetPreviousToken();

  auto pat = ParsePattern();

  Consume(lex::TokenType::ASSIGN);

  auto value = ParseAssignment();

  Consume(lex::TokenType::ELSE);

  auto else_rest = ParseAssignment();

  Consume(lex::TokenType::SEMICOLON);

  auto rest = ParseSeqExpression();

  return new LetExpression(let, pat, value, else_rest, rest);
}

////////////////////////////////////////////////////////////////////

using MemberVector = std::vector<CompoundInitializerExpr::Member>;

auto Parser::ParseDesignatedList() -> MemberVector {
  MemberVector initializers;

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

Expression* Parser::ParseParentheses() {
  if (!Matches(lex::TokenType::LEFT_PAREN)) {
    return nullptr;
  }

  if (Matches(lex::TokenType::RIGHT_PAREN)) {
    auto token = lexer_.GetPreviousToken();
    auto unit = lex::Token::UnitToken(token);
    return new LiteralExpression(unit);
  }

  auto expr = ParseExpression();
  Consume(lex::TokenType::RIGHT_PAREN);

  return expr;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseGrouping() {
  if (auto comp_expr = ParseSignleFieldCompound()) {
    return comp_expr;
  }

  if (auto block = ParseBlockExpression()) {
    return block;
  }

  if (auto paren = ParseParentheses()) {
    return paren;
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParseSignleFieldCompound() {
  if (!Matches(lex::TokenType::DOT)) {
    return nullptr;
  }

  auto dot = lexer_.GetPreviousToken();

  Consume(lex::TokenType::IDENTIFIER);
  auto ident = lexer_.GetPreviousToken();
  auto expr = TagOnly() ? nullptr : ParseExpression();

  MemberVector members = {{ident.GetName(), expr}};

  return new CompoundInitializerExpr{dot, std::move(members)};
}

////////////////////////////////////////////////////////////////////

Expression* Parser::ParsePrimary() {
  if (auto keyword_expr = ParseKeywordExpresssion()) {
    return keyword_expr;
  }

  if (auto group_expr = ParseGrouping()) {
    return group_expr;
  }

  auto token = lexer_.Peek();

  switch (token.type) {
    case lex::TokenType::INTEGER:
    case lex::TokenType::STRING:
    case lex::TokenType::FALSE:
    case lex::TokenType::TRUE:
    case lex::TokenType::CHAR:
    case lex::TokenType::UNIT:
      lexer_.Advance();
      return new LiteralExpression{token};

    case lex::TokenType::IDENTIFIER:
      Consume(lex::TokenType::IDENTIFIER);
      return new VarAccessExpression{token};

    default:
      throw parse::errors::ParsePrimaryError{token.location.Format()};
  }
}

////////////////////////////////////////////////////////////////////
