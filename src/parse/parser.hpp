#pragma once

#include <parse/parse_error.hpp>

#include <ast/statements.hpp>

#include <lex/lexer.hpp>

class Parser {
 public:
  Parser(lex::Lexer l) : lexer_{l} {
  }

  Expression* ParseExpression() {
    return ParseComparison();
  }

  ///////////////////////////////////////////////////////////////////

  Statement* ParseStatement() {
    if (auto var_decl = ParseVarDeclStatement()) {
      return var_decl;
    }

    if (auto fun_decl = ParseFunDeclStatement()) {
      return fun_decl;
    }

    if (auto ret_stmt = ParseReturnStatement()) {
      return ret_stmt;
    }

    if (auto yield_stmt = ParseYieldStatement()) {
      return yield_stmt;
    }

    if (auto expr_stmt = ParseExprStatement()) {
      return expr_stmt;
    }

    // TODO: WHILE statemnt

    std::abort();
  }

  ///////////////////////////////////////////////////////////////////

  // TODO: why different?
  FunDeclStatement* ParseFunDeclStatement();
  ReturnStatement* ParseReturnStatement();
  YieldStatement* ParseYieldStatement();
  VarDeclStatement* ParseVarDeclStatement();
  ExprStatement* ParseExprStatement();

  ///////////////////////////////////////////////////////////////////

  // clang-format off
  ///////////////////////////////////////////////////////////////////
  //                                                               //
  //   This area is used for literate prgramming                   //
  //                         ===================                   //
  //        Use it however you wish                                //
  //        -----------------------                                //
  //                                                               //
  //             ~~ Old stuff omitted ~~                           //
  //                                                               //
  //   Right now I cannot even do                                  //
  //                                                               //
  //            if true -3 else 2 + 3                              //
  //                                                               //
  //   However I can do:   - if true 3 else { 2 + 3 }              //
  //             ---                                               //
  //   I am not sure what to think about it. One one hand not      //
  //    being able to write like 1. feels strange, on the other    //
  //         it's probably a good idea to delimit both clauses     //
  //    with the {}                                                //
  //                                                               //
  //                                                               //
  ///////////////////////////////////////////////////////////////////
  // clang-format on

  ////////////////////////////////////////////////////////////////////

  Expression* ParseComparison();
  Expression* ParseBinary();
  Expression* ParseUnary();
  Expression* ParseIfExpression();
  Expression* ParseBlockExpression();
  Expression* ParseFunApplication();
  Expression* ParsePrimary();

  ////////////////////////////////////////////////////////////////////

 private:
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

  void AssertParsed(TreeNode* node, const char* error_msg) {
    if (node == nullptr) {
      throw ParseError{error_msg};
    }
  }

 private:
  lex::Lexer lexer_;
};
