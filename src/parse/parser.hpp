#pragma once

#include <parse/parse_error.hpp>

#include <ast/statements.hpp>

#include <types/repr/struct_type.hpp>
#include <types/repr/builtins.hpp>
#include <types/repr/fn_type.hpp>

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
    if (auto strcuct_decl = ParseStructDeclStatement()) {
      return strcuct_decl;
    }

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
  StructDeclStatement* ParseStructDeclStatement();
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
  //             ~~ Old stuff omitted ~~   (II)                    //
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
  Expression* ParseFunctionLike(lex::Token);
  Expression* ParsePrimary();

  ////////////////////////////////////////////////////////////////////

  types::Type* ParseType() {
    types::Type* result = nullptr;
    auto token = lexer_.Peek();

    switch (token.type) {
      case lex::TokenType::TY_INT:
        result = &types::builtin_int;
        break;

      case lex::TokenType::TY_BOOL:
        result = &types::builtin_bool;
        break;

      case lex::TokenType::TY_STRING:
        result = &types::builtin_string;
        break;

      case lex::TokenType::TY_UNIT:
        result = &types::builtin_unit;
        break;

      // Syntax: (Int, Int) -> Unit
      //         () -> Unit
      //         ((Int) -> Bool, String) -> Unit
      //          -------------  ------
      case lex::TokenType::LEFT_BRACE: {
        Consume(lex::TokenType::LEFT_BRACE);

        std::vector<types::Type*> args;
        while (auto type = ParseType()) {
          args.push_back(type);

          if (!Matches(lex::TokenType::COMMA)) {
            break;
          }
        }

        Consume(lex::TokenType::RIGHT_BRACE);

        auto return_type = ParseType();

        return new types::FnType{std::move(args), return_type};
      }

      case lex::TokenType::IDENTIFIER: {
        Consume(lex::TokenType::IDENTIFIER);
        return new types::StructType{token.GetName()};
      }

      default:
        return nullptr;
    }

    // Advance for simple types
    lexer_.Advance();
    return result;
  }

  ////////////////////////////////////////////////////////////////////

 private:
  Expression* SwitchOnId();

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
