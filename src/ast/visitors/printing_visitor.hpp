#pragma once

#include <ast/visitors/template_visitor.hpp>
#include <ast/visitors/evaluator.hpp>

#include <ast/syntax_tree.hpp>

#include <fmt/format.h>

class PrintingVisitor : public ReturnVisitor<std::string> {
 public:
  virtual void VisitStatement(Statement*) override {
    std::abort();
  }

  virtual void VisitVarDecl(VarDeclStatement* /* node */) override {
    std::abort();
  }

  virtual void VisitFunDecl(FunDeclStatement*) override {
    std::abort();
  }

  virtual void VisitExprStatement(ExprStatement*) override {
    std::abort();
  }

  virtual void VisitReturn(ReturnStatement*) override {
    std::abort();
  }

  virtual void VisitYield(YieldStatement*) override {
    std::abort();
  }

  virtual void VisitExpression(Expression*) override {
    std::abort();
  }

  virtual void VisitComparison(ComparisonExpression* node) override {
    return_value = fmt::format(
        "Comparison-of-(\n"
        "\t type:    {}  \n"
        "\t operand: {}) \n"                         //
        "\t operand: {}) \n",                        //
        lex::FormatTokenType(node->operator_.type),  //
        Eval(node->left_), Eval(node->right_));
  };

  virtual void VisitBinary(BinaryExpression* node) override {
    return_value = fmt::format(
        "Binary-of-(\n"
        "\t type:    {}  \n"
        "\t operand: {}) \n"                         //
        "\t operand: {}) \n",                        //
        lex::FormatTokenType(node->operator_.type),  //
        Eval(node->left_), Eval(node->right_));
  }

  virtual void VisitUnary(UnaryExpression* node) override {
    return_value = fmt::format(
        "Unary-of-(\n"
        "\t type:    {}  \n"
        "\t operand: {}) \n",                        //
        lex::FormatTokenType(node->operator_.type),  //
        Eval(node->operand_));
  }

  virtual void VisitIf(IfExpression*) override {
    std::abort();
  }

  virtual void VisitBlock(BlockExpression*) override {
    std::abort();
  }

  virtual void VisitFnCall(FnCallExpression*) override {
    std::abort();
  }

  virtual void VisitLiteral(LiteralExpression* node) override {
    auto lit_string = Format(literal_eval_.Eval(node));
    return_value = fmt::format("Literal {}", lit_string);
  }

  virtual void VisitLvalue(VarAccessExpression* node) override {
    auto lit_string = Format(literal_eval_.Eval(node));
    return_value = fmt::format("Literal {}", lit_string);
  }

 private:
  Evaluator literal_eval_;
};
