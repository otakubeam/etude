#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/ir_emitter.hpp>

#include <ast/visitors/abort_visitor.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <fmt/format.h>

#include <unordered_map>
#include <utility>

namespace qbe {

class GenAt : public AbortVisitor {
 public:
  // Target id is a pointer
  GenAt(IrEmitter& parent, size_t target_id)
      : parent_{parent}, target_id_{target_id} {
  }

  std::string Give() {
    return result_;
  }

  virtual void VisitDeref(DereferenceExpression* node) override {
    (void)node;
    std::abort();
  }

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
    (void)node;
    std::abort();
  }

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override {
    for (auto& i : node->initializers_) {
      auto offset =
          parent_.measure_.MeasureFieldOffset(node->GetType(), i.field);

      auto with_offset = parent_.id_ += 1;
      result_ += fmt::format("  %.{} =l add %.{}, %.{}\n", with_offset,
                             target_id_, offset);
      GenAt tmp(parent_, with_offset);
      i.init->Accept(&tmp);

      result_ += tmp.Give();
    }
  }

  virtual void VisitVarAccess(VarAccessExpression* node) override {
    auto id = parent_.Eval(node);
    FMT_ASSERT(parent_.GetTypeSize(node->GetType()) == 4, "Unsupported size");
    result_ += fmt::format("  storew %.{}, %.{}\n", id, target_id_);
  }

  virtual void VisitLiteral(LiteralExpression* node) override {
    auto id = parent_.Eval(node);
    result_ += fmt::format("  storew %.{}, %.{}\n", id, target_id_);
  }

 private:
  IrEmitter& parent_;
  size_t target_id_;
  std::string result_;
};

}  // namespace qbe
