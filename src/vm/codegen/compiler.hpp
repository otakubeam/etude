#pragma once

#include <vm/codegen/frame_translator.hpp>
#include <vm/chunk.hpp>

#include <ast/visitors/template_visitor.hpp>
#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <algorithm>
#include <ranges>

namespace vm::codegen {

class Compiler : public Visitor {
 public:
  virtual ~Compiler();

  ExecutableChunk Compile(TreeNode* node) {
    node->Accept(this);
    return chunk_;
  }

  static std::vector<ExecutableChunk>* CompileScript(TreeNode* node) {
    auto result = new std::vector<ExecutableChunk>;

    Compiler c;
    c.compiled_chunks_ = result;
    c.current = new FrameTranslator{};

    node->Accept(&c);

    result->push_back(c.chunk_);

    return result;
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

  virtual void VisitFunDecl(FunDeclStatement* node) override {
    FrameTranslator builder{node};

    Compiler chunk_compiler;
    chunk_compiler.current = &builder;
    // For the case of nested fn declarations
    chunk_compiler.compiled_chunks_ = compiled_chunks_;

    auto chunk = chunk_compiler.Compile(node->block_);

    int chunk_no = compiled_chunks_->size();
    uint8_t const_no = chunk_.attached_vals.size();

    chunk_.attached_vals.push_back(rt::PrimitiveValue{
        .tag = rt::ValueTag::Int,
        .as_int = chunk_no,
    });

    compiled_chunks_->push_back(chunk);

    // This allows for lookup of this symbol later at the callsite
    current->AddLocal(node->name_.GetName());

    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::PUSH_STACK,
        .arg1 = const_no,  // push the constant of the compiled chunk
    });
  }

  virtual void VisitFnCall(FnCallExpression* node) override {
    // Place in reverse order
    for (int i = node->arguments_.size() - 1; i >= 0; i -= 1) {
      node->arguments_[i]->Accept(this);
    }

    auto mb_offset = current->Lookup(node->fn_name_.GetName());

    int offset = mb_offset.value();

    // TODO: branch direct / indirect

    GenerateVarFetch(offset);

    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::INDIRECT_CALL,
    });
  }

 private:
  void GenerateVarFetch(int offset) {
    if (offset > 0) {
      chunk_.instructions.push_back(vm::Instr{
          .type = vm::InstrType::GET_LOCAL,
          .arg1 = (uint8_t)offset,
      });
    } else {
      chunk_.instructions.push_back(vm::Instr{
          .type = vm::InstrType::GET_ARG,
          .arg1 = (uint8_t)(-offset),
      });
    }
  }

 public:
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
    /////////

    node->condition_->Accept(this);

    auto jump_to_false_ip = chunk_.instructions.size();

    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::JUMP_IF_FALSE,
    });

    /////////

    node->true_branch_->Accept(this);

    auto jump_to_end_ip = chunk_.instructions.size();

    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::JUMP,
    });

    /////////

    auto false_ip_start = chunk_.instructions.size();

    node->false_branch_->Accept(this);

    auto false_ip_end = chunk_.instructions.size();

    // Backpatch JUMP_IF_FALSE
    chunk_.instructions.at(jump_to_false_ip) = vm::Instr{
        .type = InstrType::JUMP_IF_FALSE,
        .arg2 = (uint8_t)(false_ip_start >> 8),
        .arg3 = (uint8_t)(false_ip_start),
    };

    // Backpatch JUMP
    chunk_.instructions.at(jump_to_end_ip) = vm::Instr{
        .type = InstrType::JUMP,
        .arg2 = (uint8_t)(false_ip_end >> 8),
        .arg3 = (uint8_t)(false_ip_end),
    };
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBlock(BlockExpression* node) override {
    for (auto& st : node->stmts_) {
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

  virtual void VisitVarAccess(VarAccessExpression* node) override {
    auto mb_offset = current->Lookup(node->name_.GetName());
    int offset = mb_offset.value();
    GenerateVarFetch(offset);
  }

  ////////////////////////////////////////////////////////////////////

 private:
  // Compiler(std::vector<ExecutableChunk>& chnks) : compiled_chunks_{chnks} {
  // }

 private:
  ExecutableChunk chunk_;

  // Environment
  std::vector<ExecutableChunk>* compiled_chunks_ = nullptr;

  // StackEmulation
  FrameTranslator* current = nullptr;
};

}  // namespace vm::codegen
