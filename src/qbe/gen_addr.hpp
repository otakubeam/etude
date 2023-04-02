#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>
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
  GenAddr(IrEmitter& parent, Value target_id)
      : parent_{parent}, target_id_{target_id} {
  }

  virtual void VisitDeref(DereferenceExpression* node) override {
    fmt::print("  {} =l copy {}\n", target_id_.Emit(),
               parent_.Eval(node->operand_).Emit());
  }

  virtual void VisitFnCall(FnCallExpression* node) override {
    fmt::print("  {} =l copy {}\n", target_id_.Emit(),
               parent_.Eval(node).Emit());
  }

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
    node->struct_expression_->Accept(this);

    auto offset = parent_.measure_.MeasureFieldOffset(
        node->struct_expression_->GetType(), node->field_name_);

    fmt::print("  {} =l add {}, {}\n",  //
               target_id_.Emit(), target_id_.Emit(), offset);
  }

  virtual void VisitVarAccess(VarAccessExpression* node) override {
    fmt::print("  {} =l copy {}\n",  //
               target_id_.Emit(),
               parent_.named_values_.at(node->GetName()).Emit());
  }

 private:
  IrEmitter& parent_;
  Value target_id_;
};

}  // namespace qbe
