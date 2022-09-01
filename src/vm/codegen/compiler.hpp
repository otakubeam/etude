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

  virtual void VisitVarDecl(VarDeclStatement*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructDecl(StructDeclStatement*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    // rt::SBObject value;
  };

  virtual void VisitReturn(ReturnStatement*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitYield(YieldStatement*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement* node) override {
    VisitExpression(node->expr_);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  virtual void VisitComparison(ComparisonExpression*) override {
    FMT_ASSERT(false, "Unimplemented!");
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

  virtual void VisitUnary(UnaryExpression*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitIf(IfExpression* node) override {
    node->condition_->Accept(this);
    auto jump_ip = chunk_.instructions.size();

    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::JUMP_IF_FALSE,
    });

    node->true_branch_->Accept(this);

    // TODO: jump to end

    auto false_ip_start = chunk_.instructions.size();

    // TODO: generate false branch

    // Backpatch
    chunk_.instructions.at(jump_ip) = vm::Instr{
        .type = InstrType::JUMP_IF_FALSE,
        .arg2 = (uint8_t)(false_ip_start >> 8),
        .arg3 = (uint8_t)(false_ip_start),
    };
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBlock(BlockExpression* node) override {
    for (auto& st : node->stmts_) {
      FMT_ASSERT(false, "Unimplemented!");
      st->Accept(this);
    }

    if (node->final_) {
      node->final_->Accept(this);
      return;
    }

    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::PUSH_STACK,
        // XXX: this is obviously wrong
        .arg1 = 0,  // push nil instead
    });
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructConstruction(StructConstructionExpression*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFieldAccess(FieldAccessExpression*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFnCall(FnCallExpression*) override {
    FMT_ASSERT(false, "Unimplemented!");
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

  virtual void VisitVarAccess(VarAccessExpression*) override {
    FMT_ASSERT(false, "Unimplemented!");
  }

  ////////////////////////////////////////////////////////////////////

 private:
  ExecutableChunk chunk_;

  // Environment

  // StackEmulation
};

}  // namespace vm::codegen
