#pragma once

#include <parse/parsing_errors.hpp>

#include <ast/declarations.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer& l);

  ////////////////////////////////////////////////////////////////////
  //                            Module                              //
  ////////////////////////////////////////////////////////////////////

  class ModuleBuilder {
   public:
    ModuleBuilder(Parser& me) : me(me) {
    }

    auto Run() -> ModuleDeclaration*;

   private:
    auto ParseImports() -> void;
    auto ParseExportBlock() -> void;
    auto ParseRestDefinitions() -> void;

    Parser& me;
    ModuleDeclaration* result;
  };

  auto ParseModule(std::string_view name) -> ModuleDeclaration*;

  ////////////////////////////////////////////////////////////////////
  //                        Declarations                            //
  ////////////////////////////////////////////////////////////////////

  Declaration* ParseDeclaration();

  TraitDeclaration* ParseTraitDeclaration();  // trait
  ImplDeclaration* ParseImplDeclaration();    // impl
  Declaration* ParseAssociatedItems();        // fun | var | type

  TypeDeclaration* ParseTypeDeclaration();
  VarDeclaration* ParseVarDeclaration(types::Type* hint);
  FunDeclaration* ParseFunDeclaration(types::Type* hint);

  Attribute* ParseAttributes();
  FunDeclaration* ParseFunPrototype(types::Type* hint);
  FunDeclaration* ParseFunDeclarationStandalone();

  ////////////////////////////////////////////////////////////////////
  //                        Expressions                             //
  ////////////////////////////////////////////////////////////////////

  // Top

  Expression* ParseExpression();  // -> ParseAssignment();

  // Sequencing:
  //
  // These expressions are somewhat special for they
  // are only allowed inside `GroupingExpressions`.
  //

  // <expr> ; <expr>
  Expression* ParseSeqExpression();

  // let <pat> = <expr> else <expr> ; <seq_expr>
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
  Expression* ParseFieldAccess(Expression* expr);  // .id
  Expression* ParseArrow(Expression* expr);        // ->id
  Expression* ParseIndexing(Expression* expr);     // [<expr>]
  Expression* ParseCall(Expression* expr);         // ( <expr>,* )
  Expression* ParseCast(Expression* expr);         // ~> <type>

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
  //                           Patterns                             //
  ////////////////////////////////////////////////////////////////////

  Pattern* ParsePattern();
  Pattern* ParseLiteralPattern();
  Pattern* ParseBindingPattern();
  Pattern* ParseDiscardingPattern();
  Pattern* ParseVariantPattern();

  ////////////////////////////////////////////////////////////////////

  types::Type* ParseType();
  types::Type* ParseFunctionType();
  types::Type* ParsePointerType();
  types::Type* ParseStructType();
  types::Type* ParseSumType();
  types::Type* ParseTyApp();
  types::Type* ParseTyGrouping();
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
