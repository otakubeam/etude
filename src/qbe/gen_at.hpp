#pragma once

#include <qbe/ir_emitter.hpp>
#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>

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
  GenAt(IrEmitter& parent, Value target_id)
      : parent_{parent}, target_id_{target_id} {
  }

  std::string Give() {
    return result_;
  }

  virtual void VisitDeref(DereferenceExpression* node) override {
    auto id = parent_.Eval(node);
    result_ += fmt::format("  store{} {}, {}\n", StoreSuf(node->GetType()),
                           id.Emit(), target_id_.Emit());
  }

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
    (void)node;
    std::abort();
  }

  virtual void VisitFnCall(FnCallExpression* node) override {
    auto call = parent_.Eval(node);
    result_ += fmt::format("  store{} {}, {}\n", ToQbeType(node->GetType()),
                           call.Emit(), target_id_.Emit());
  }

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override {
    for (auto& i : node->initializers_) {
      auto offset =
          parent_.measure_.MeasureFieldOffset(node->GetType(), i.field);

      // Move the pointer
      result_ += fmt::format("  {} =l add {}, {}\n", target_id_.Emit(),
                             target_id_.Emit(), offset);
      // Generate result there
      i.init->Accept(this);
    }
  }

  virtual void VisitNew(NewExpression* node) override {
    auto mem = parent_.Eval(node);
    fmt::print("  storel {}, {}\n", mem.Emit(), target_id_.Emit());
  }

  virtual void VisitVarAccess(VarAccessExpression* node) override {
    auto id = parent_.Eval(node);
    FMT_ASSERT(parent_.GetTypeSize(node->GetType()) == 4, "Unsupported size");
    result_ += fmt::format("  storew {}, {}\n", id.Emit(), target_id_.Emit());
  }

  virtual void VisitLiteral(LiteralExpression* node) override {
    auto id = parent_.Eval(node);
    result_ += fmt::format("  store{} {}, {}\n", StoreSuf(node->GetType()),
                           id.Emit(), target_id_.Emit());
  }

 private:
  IrEmitter& parent_;
  Value target_id_;
  std::string result_;
};

}  // namespace qbe
