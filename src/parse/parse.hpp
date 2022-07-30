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

  FunDeclStatement* ParseFunDeclStatement() {
    if (!Matches(lex::TokenType::FUN)) {
      return nullptr;
    }

    auto fun_name = lexer_.Peek();
    Consume(lex::TokenType::IDENTIFIER);
    Consume(lex::TokenType::LEFT_BRACE);

    //// TODO: separate into fn GetFormals()
    std::vector<lex::Token> formals;
    auto formal_param = lexer_.Peek();

    while (Matches(lex::TokenType::IDENTIFIER)) {
      formals.push_back(formal_param);
      if (!Matches(lex::TokenType::COMMA)) {
        break;
      }
      formal_param = lexer_.Peek();
    }
    //// separate into fn GetFormals()

    Consume(lex::TokenType::RIGHT_BRACE);

    auto block = ParseBlockStatement();
    return new FunDeclStatement{fun_name, std::move(formals), block};
  }

  ///////////////////////////////////////////////////////////////////

  BlockStatement* ParseBlockStatement() {
    if (!Matches(lex::TokenType::LEFT_CBRACE)) {
      return nullptr;
    }

    std::vector<Statement*> stmts;

    while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
      auto stmt = ParseStatement();
      stmts.push_back(stmt);
    }

    return new BlockStatement{std::move(stmts)};
  }

  ///////////////////////////////////////////////////////////////////

  IfStatement* ParseIfStatement() {
    if (!Matches(lex::TokenType::IF)) {
      return nullptr;
    }

    // This should be fine even without parentheses, right?
    auto condition = ParseExpression();
    FMT_ASSERT(condition,  //
               "If statement without condition");

    auto true_branch = ParseBlockStatement();
    FMT_ASSERT(true_branch,  //
               "If statement without true branch");

    BlockStatement* false_branch = nullptr;

    if (Matches(lex::TokenType::ELSE)) {
      false_branch = ParseBlockStatement();
      FMT_ASSERT(false_branch,  //
                 "Else clause without an associated statement");
    }

    return new IfStatement(condition, true_branch, false_branch);
  }

  ///////////////////////////////////////////////////////////////////

  ReturnStatement* ParseReturnStatement() {
    if (!Matches(lex::TokenType::RETURN)) {
      return nullptr;
    }

    Expression* ret_expr = nullptr;
    if (!Matches(lex::TokenType::SEMICOLUMN)) {
      ret_expr = ParseExpression();
      Consume(lex::TokenType::SEMICOLUMN);
    }

    return new ReturnStatement{ret_expr};
  }

  ///////////////////////////////////////////////////////////////////

  // Doesn't this sound a bit like `break`?

  YieldStatement* ParseYieldStatement() {
    if (!Matches(lex::TokenType::YIELD)) {
      return nullptr;
    }

    Expression* ret_expr = nullptr;
    if (!Matches(lex::TokenType::SEMICOLUMN)) {
      ret_expr = ParseExpression();
      Consume(lex::TokenType::SEMICOLUMN);
    }

    return new YieldStatement{ret_expr};
  }

  ///////////////////////////////////////////////////////////////////

  VarDeclStatement* ParseVarDeclStatement() {
    if (!Matches(lex::TokenType::VAR)) {
      return nullptr;
    }

    // 1. Get a name to assign to

    auto token = lexer_.Peek();

    Consume(lex::TokenType::IDENTIFIER);
    auto lvalue = new LiteralExpression{std::move(token)};

    // 2. Get an expression to assign to

    Consume(lex::TokenType::ASSIGN);

    auto value = ParseExpression();
    FMT_ASSERT(value, "Trying to assign a non-existent value");

    Consume(lex::TokenType::SEMICOLUMN);

    return new VarDeclStatement{lvalue, value};
  }

  ///////////////////////////////////////////////////////////////////

  ExprStatement* ParseExprStatement() {
    auto expr = ParseExpression();
    Consume(lex::TokenType::SEMICOLUMN);

    return new ExprStatement{expr};
  }

  // clang-format off
  ///////////////////////////////////////////////////////////////////
  //                                                               //
  //   This area is used for literate prgramming                   //
  //                         ===================                   //
  //        Use it however you wish                                //
  //        -----------------------                                //
  //                                                               //
  //   The second big thing I need to do is to          (a)        //
  //   be able to parse statements.                                //
  //                    ----------                                 //
  //                                                               //
  //             - Specifically IF statement and VAR statement     //
  //                            ============     =============     //
  //                                                               //
  //    I can parse them now! Great. The only thing left is to     //
  //          ---------------        test the implementation.      //
  //                                                               //
  //    And of course I still need to fix visitors to recognize    //
  //     these new statements.     ---------------                 //
  //                                                               //
  //                                                               //
  //    It seems to be more useful to make 'if' an expression.     //
  //                               ============ --------------     //
  //    I wonder if there are any unintended consequences to that  //
  //                              -----------------------          //
  //                                                               //
  //                                                               //
  //                                                               //
  //                                                               //
  //                                                               //
  //                                                               //
  ///////////////////////////////////////////////////////////////////
  // clang-format on

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
    FMT_ASSERT(Matches(type), error_msg.c_str());
  }

 private:
  lex::Lexer lexer_;
};
