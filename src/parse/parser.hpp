#pragma once

#include <parse/parse_error.hpp>

#include <ast/statements.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer l) : lexer_{l} {
  }

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

  Expression* ParseComparison();
  Expression* ParseBinary();
  Expression* ParseUnary();
  Expression* ParseFieldAccess(LvalueExpression* expr);
  Expression* ParseIfExpression();
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
  auto ParseFormals()  //
      -> std::vector<FunDeclStatement::FormalParam>;

  Expression* SwitchOnId();

  std::vector<Expression*> ParseCSV();

  bool Matches(lex::TokenType type) {
    if (lexer_.Peek().type != type) {
      return false;
    }

    lexer_.Advance();
    return true;
  }

  void Consume(lex::TokenType type) {
    auto error_msg = fmt::format("\nCould not match type {}\n",  //
                                 lex::FormatTokenType(type));
    if (!Matches(type)) {
      throw ParseError{error_msg.c_str()};
    }
  }

  void AssertParsed(void* node, const char* error_msg) {
    if (node == nullptr) {
      throw ParseError{error_msg};
    }
  }

 private:
  lex::Lexer lexer_;
};
