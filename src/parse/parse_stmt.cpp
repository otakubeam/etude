#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

///////////////////////////////////////////////////////////////////

Statement* Parser::ParseStatement() {
  if (auto expression_statement = ParseExprStatement()) {
    return expression_statement;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////


Statement* Parser::ParseExprStatement() {
  auto expr = ParseExpression();

  if (Matches(lex::TokenType::ASSIGN) ||
      Matches(lex::TokenType::PLUS_EQ) || Matches(lex::TokenType::MINUS_EQ) ||
      Matches(lex::TokenType::STAR_EQ) || Matches(lex::TokenType::DIV_EQ)) {
    if (auto target = expr->as<LvalueExpression>()) {
      return ParseAssignment(target);
    }

    throw parse::errors::ParseNonLvalueError{FormatLocation()};
  }

  try {
    Consume(lex::TokenType::SEMICOLON);
  } catch (...) {
    // So that the last expression in block can be caught
    //   {
    //     stmt;
    //     stmt;   <<<--- parse all of it as statements
    //     expr           but catch the last one
    //   }
    //
    throw new ExprStatement{expr};
  }

  return new ExprStatement{expr};
}

///////////////////////////////////////////////////////////////////

AssignmentStatement* Parser::ParseAssignment(LvalueExpression* target) {
  auto assignment_loc = lexer_.GetPreviousToken();
  auto value = ParseExpression();
  Consume(lex::TokenType::SEMICOLON);
  return new AssignmentStatement{assignment_loc, target, value};
}

///////////////////////////////////////////////////////////////////
