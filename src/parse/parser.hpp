#pragma once

#include <driver/module.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer& l);

  auto ParseModule() -> Module;

  ////////////////////////////////////////////////////////////////////
  //                           Patterns                             //
  ////////////////////////////////////////////////////////////////////

  Pattern* ParsePattern();
  Pattern* ParseLiteralPattern();
  Pattern* ParseBindingPattern();
  Pattern* ParseDiscardingPattern();
  Pattern* ParseVariantPattern();

  ////////////////////////////////////////////////////////////////////
  //                        Declarations                            //
  ////////////////////////////////////////////////////////////////////

  Declaration* ParseDeclaration();

  TraitDeclaration* ParseTraitDeclaration();
  ImplDeclaration* ParseImplDeclaration();
  TypeDeclaration* ParseTypeDeclaration();
  FunDeclaration* ParseFunDeclaration(types::Type* hint);
  VarDeclaration* ParseVarDeclaration(types::Type* hint);

  Attribute* ParseAttributes();
  Declaration* ParsePrototype(bool require_sigature = false);
  FunDeclaration* ParseFunPrototype(types::Type* hint);
  FunDeclaration* ParseFunDeclarationStandalone();

  ////////////////////////////////////////////////////////////////////
  //                        Expressions                             //
  ////////////////////////////////////////////////////////////////////

  // Top

  Expression* ParseExpression();

  Expression* ParseSeqExpression();
  Expression* ParseLetExpression();

  // Arithmetic

  Expression* ParseAssignment();      // =
  Expression* ParseComparison();      // == <= >= != < >
  Expression* ParseAdditive();        // + -
  Expression* ParseMultiplicative();  // * \ %
  Expression* ParseUnary();           // ! -
  Expression* ParseDeref();           // *
  Expression* ParseAddressof();       // &

  // Postfix Expresssions

  Expression* ParsePostfixExpressions();
  Expression* ParseIndirectFieldAccess(Expression* expr);
  Expression* ParseIndexingExpression(Expression* expr);
  Expression* ParseFieldAccess(Expression* expr);
  Expression* ParseFnCall(Expression* expr);
  Expression* ParseCast(Expression* expr);

  // Blocks / Grouping

  Expression* ParseBlockExpression();  // ( <expr> ) and { ... }
  Expression* ParseParentheses();      // ( <expr> )
  Expression* ParseGrouping();         // { ... }

  Expression* ParseCompoundInitializer(lex::Token curly_brace);
  Expression* ParseSignleFieldCompound();

  // Keywords

  Expression* ParseKeywordExpresssion();
  Expression* ParseReturnExpression();
  Expression* ParseYieldExpression();
  Expression* ParseMatchExpression();
  Expression* ParseNewExpression();
  Expression* ParseIfExpression();

  // Bottom

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
  auto ParseDesignatedList() -> std::vector<CompoundInitializerExpr::Member>;

  bool MatchesAssignmentSign(lex::TokenType type);
  bool MatchesComparisonSign(lex::TokenType type);
  bool Matches(lex::TokenType type);
  void Consume(lex::TokenType type);
  bool TagOnly();

  std::string FormatLocation();

 private:
  lex::Lexer& lexer_;
};
