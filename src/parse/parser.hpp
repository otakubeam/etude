#pragma once

#include <ast/statements.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer& l);

  // Top-level
  // Parses all declarations in a file
  auto ParseUnit() -> std::vector<Statement*>;

  ///////////////////////////////////////////////////////////////////

  Statement* ParseStatement();

  ReturnStatement* ParseReturnStatement();
  YieldStatement* ParseYieldStatement();
  Statement* ParseExprStatement();
  AssignmentStatement* ParseAssignment(LvalueExpression* target);

  ////////////////////////////////////////////////////////////////////

  Statement* ParseDeclaration();

  StructDeclStatement* ParseStructDeclStatement();
  FunDeclStatement* ParseFunDeclStatement();
  VarDeclStatement* ParseVarDeclStatement();

  ////////////////////////////////////////////////////////////////////

  Expression* ParseExpression();

  Expression* ParseDeref();
  Expression* ParseAddressof();
  Expression* ParseComparison();
  Expression* ParseBinary();
  Expression* ParseUnary();
  Expression* ParseIfExpression();
  Expression* ParseNewExpression();
  Expression* ParseBlockExpression();

  // Precedence 1
  Expression* ParseTypecastExpression();
  Expression* ParseFieldAccess(Expression* expr);
  Expression* ParseFnCallExpression(lex::Token id);
  Expression* ParseConstructionExpression(lex::Token id);

  Expression* ParsePrimary();

  ////////////////////////////////////////////////////////////////////

  types::Type* ParseFunctionType();
  types::Type* ParsePointerType();
  types::Type* ParseStructType();
  types::Type* ParsePrimitiveType();

  ////////////////////////////////////////////////////////////////////

 private:
  auto ParseFormals() -> std::vector<FunDeclStatement::FormalParam>;
  auto ParseCSV() -> std::vector<Expression*>;

  bool Matches(lex::TokenType type);
  bool MatchesComparisonSign(lex::TokenType type);
  void Consume(lex::TokenType type);

  std::string FormatLocation();

 private:
  lex::Lexer& lexer_;
};
