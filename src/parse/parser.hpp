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
    if (auto if_stmt = ParseIfStatement()) {
      return if_stmt;
    }

    if (auto var_decl = ParseVarDeclStatement()) {
      return var_decl;
    }

    if (auto fun_decl = ParseFunDeclStatement()) {
      return fun_decl;
    }

    if (auto block_statement = ParseBlockStatement()) {
      return block_statement;
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

  FunDeclStatement* ParseFunDeclStatement();
  BlockStatement* ParseBlockStatement();
  IfStatement* ParseIfStatement();
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
  //                                                               //
  ///////////////////////////////////////////////////////////////////
  // clang-format on

  ////////////////////////////////////////////////////////////////////

  Expression* ParseComparison();
  Expression* ParseBinary();
  Expression* ParseUnary();
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
