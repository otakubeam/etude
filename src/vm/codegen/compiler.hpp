#pragma once

#include <vm/chunk.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

namespace vm::codegen {

class Compiler : public Visitor {
 public:
  virtual ~Compiler();

  ExecutableChunk Compile(TreeNode* node) {
    node->Accept(this);
    return chunk_;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStatement(Statement* /* node */) override {
    FMT_ASSERT(false, "Visiting bare statement");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructDecl(StructDeclStatement* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    // rt::SBObject value;
  };

  virtual void VisitReturn(ReturnStatement* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitYield(YieldStatement* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement* node) override {
    VisitExpression(node->expr_);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression* node) override {
  }

  virtual void VisitComparison(ComparisonExpression* node) override {
  }

  virtual void VisitBinary(BinaryExpression* node) override {
    node->left_->Accept(this);
    node->right_->Accept(this);

    switch (node->operator_.type) {
      case lex::TokenType::PLUS:
        chunk_.instructions.push_back(vm::Instr{
            .type = InstrType::ADD,
        });
        break;

      case lex::TokenType::MINUS:
        FMT_ASSERT(false, "Unimplemented!");

      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  virtual void VisitUnary(UnaryExpression* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitIf(IfExpression* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBlock(BlockExpression* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructConstruction(
      StructConstructionExpression* s_cons) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFnCall(FnCallExpression* node) override {
  }

  virtual void VisitLiteral(LiteralExpression* lit) override {
    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::PUSH_STACK,
        .arg1 = (uint8_t)chunk_.attached_vals.size(),
    });

    switch (lit->token_.type) {
      case lex::TokenType::NUMBER: {
        chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
            .tag = rt::ValueTag::Int,
            .as_int = std::get<int>(lit->token_.sem_info),
        });

        break;
      }

      case lex::TokenType::STRING:
        FMT_ASSERT(false, "Unimplemented!");

      case lex::TokenType::TRUE: {
        chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
            .tag = rt::ValueTag::Bool,
            .as_bool = true,
        });

        break;
      }

      case lex::TokenType::FALSE: {
        chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
            .tag = rt::ValueTag::Bool,
            .as_bool = false,
        });

        break;
      }

      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  virtual void VisitVarAccess(VarAccessExpression* ident) override {
  }

  ////////////////////////////////////////////////////////////////////

 private:
  ExecutableChunk chunk_;

  // Environment

  // StackEmulation
};

}  // namespace vm::codegen
