#pragma once

#include <ast/statements.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer l);

  ///////////////////////////////////////////////////////////////////

  Statement* ParseStatement();

  StructDeclStatement* ParseStructDeclStatement();
  FunDeclStatement* ParseFunDeclStatement();
  ReturnStatement* ParseReturnStatement();
  YieldStatement* ParseYieldStatement();
  VarDeclStatement* ParseVarDeclStatement();
  Statement* ParseExprStatement();
  AssignmentStatement* ParseAssignment(LvalueExpression* target);

  ////////////////////////////////////////////////////////////////////

  Expression* ParseExpression();

  Expression* ParseDeref();
  Expression* ParseAddressof();
  Expression* ParseComparison();
  Expression* ParseBinary();
  Expression* ParseUnary();
  Expression* ParseFieldAccess(LvalueExpression* expr);
  Expression* ParseIfExpression();
  Expression* ParseNewExpression();
  Expression* ParseBlockExpression();
  Expression* ParseFnCallExpression(lex::Token id);
  Expression* ParseConstructionExpression(lex::Token id);
  Expression* ParsePrimary();

  ////////////////////////////////////////////////////////////////////

  types::Type* ParseType();
  types::Type* ParseFunctionType();
  types::Type* ParseStructType();

  ////////////////////////////////////////////////////////////////////

 private:
  auto ParseFormals() -> std::vector<FunDeclStatement::FormalParam>;
  auto ParseCSV() -> std::vector<Expression*>;

  bool Matches(lex::TokenType type);
  void Consume(lex::TokenType type);

  std::string FormatLocation();

 private:
  lex::Lexer lexer_;
};
