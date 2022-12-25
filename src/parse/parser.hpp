#pragma once

#include <driver/module.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer& l);

  auto ParseModule() -> Module;

  ///////////////////////////////////////////////////////////////////

  Pattern* ParsePattern();
  Pattern* ParseLiteralPattern();
  Pattern* ParseBindingPattern();
  Pattern* ParseDiscardingPattern();
  Pattern* ParseVariantPattern();

  ///////////////////////////////////////////////////////////////////

  Statement* ParseStatement();

  ReturnStatement* ParseReturnStatement();
  YieldStatement* ParseYieldStatement();
  Statement* ParseExprStatement();
  AssignmentStatement* ParseAssignment(LvalueExpression* target);

  ////////////////////////////////////////////////////////////////////

  Declaration* ParseDeclaration();

  Attribute* ParseAttributes();
  Declaration* ParsePrototype(bool require_sigature = false);
  FunDeclStatement* ParseFunPrototype(types::Type* hint);

  TraitDeclaration* ParseTraitDeclaration();
  TypeDeclStatement* ParseTypeDeclStatement();
  FunDeclStatement* ParseFunDeclStatement(types::Type* hint);
  VarDeclStatement* ParseVarDeclStatement(types::Type* hint);

  ////////////////////////////////////////////////////////////////////

  Expression* ParseExpression();

  Expression* ParseKeywordExpresssion();

  Expression* ParseIfExpression();
  Expression* ParseMatchExpression();
  Expression* ParseNewExpression();
  Expression* ParseBlockExpression();

  Expression* ParseComparison();
  Expression* ParseBinary();

  Expression* ParseUnary();
  Expression* ParseDeref();
  Expression* ParseAddressof();

  // Precedence 1
  Expression* ParsePostfixExpressions();
  Expression* ParseFieldAccess(Expression* expr);
  Expression* ParseIndirectFieldAccess(Expression* expr);
  Expression* ParseIndexingExpression(Expression* expr);
  Expression* ParseFnCallUnnamed(Expression* expr);
  Expression* ParseFnCallExpression(Expression* expr, lex::Token id);

  Expression* ParseCompoundInitializer(lex::Token id);
  Expression* ParseSignleFieldCompound();
  Expression* ParsePrimary();

  ////////////////////////////////////////////////////////////////////

  types::Type* ParseType();
  types::Type* ParseFunctionType();
  types::Type* ParsePointerType();
  types::Type* ParseStructType();
  types::Type* ParseSumType();
  types::Type* ParsePrimitiveType();

  ////////////////////////////////////////////////////////////////////

 private:
  auto ParseCSV() -> std::vector<Expression*>;
  auto ParseFormals() -> std::vector<lex::Token>;
  auto ParseDesignatedList()  //
      -> std::vector<CompoundInitializerExpr::Member>;

  bool Matches(lex::TokenType type);
  bool MatchesComparisonSign(lex::TokenType type);
  void Consume(lex::TokenType type);
  bool TagOnly();

  std::string FormatLocation();

 private:
  lex::Lexer& lexer_;
};
