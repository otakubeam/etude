#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/ir_emitter.hpp>

#include <ast/visitors/abort_visitor.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <unordered_map>
#include <fmt/format.h>
#include <iterator>
#include <utility>

namespace qbe {

class GenAddr : public AbortVisitor {
 public:
  GenAddr(IrEmitter& parent, size_t target_id)
      : parent_{parent}, target_id_{target_id} {
  }

  std::string Give() {
    return result_;
  }

  virtual void VisitDeref(DereferenceExpression* node) override {
    node->operand_->Accept(this);
  }

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
    (void)node;
    std::abort();
  }

  virtual void VisitVarAccess(VarAccessExpression* node) override {
    result_ += fmt::format("  %.{} =w %.{}\n",  //
                           target_id_, parent_.ids_.at(node->GetName()));
  }

 private:
  IrEmitter& parent_;
  size_t target_id_;
  std::string result_;
};

}  // namespace qbe
