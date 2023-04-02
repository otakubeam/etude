#pragma once

#include <qbe/ir_emitter.hpp>
#include <qbe/gen_addr.hpp>
#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>

#include <ast/visitors/abort_visitor.hpp>

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

  virtual void VisitDeref(DereferenceExpression* node) override {
    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    auto addr = parent_.Eval(node->operand_);
    auto [s, a] = parent_.SizeAlign(node);
    parent_.Copy(a, s, addr, target_id_);
  }

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
    auto addr = parent_.GenTemporary();

    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    parent_.GenAddress(node->struct_expression_, addr);

    auto offset = parent_.measure_.MeasureFieldOffset(
        node->struct_expression_->GetType(), node->field_name_);

    fmt::print("  {} =l add {}, {}\n", addr.Emit(), addr.Emit(), offset);

    auto [s, a] = parent_.SizeAlign(node);

    parent_.Copy(a, s, addr, target_id_);
  }

  virtual void VisitFnCall(FnCallExpression* node) override {
    auto call = parent_.Eval(node);

    if (parent_.measure_.IsCompound(node->GetType())) {
      auto [s, a] = parent_.SizeAlign(node);
      parent_.Copy(a, s, call, target_id_);
      return;
    }

    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    fmt::print("  store{} {}, {}\n", StoreSuf(node->GetType()), call.Emit(),
               target_id_.Emit());
  }

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override {
    size_t previous_offset = 0;

    auto& measure = parent_.measure_;
    auto& field = node->initializers_[0].field;  // The only one!
    auto underlying = types::TypeStorage(node->GetType());

    if (underlying->tag == types::TypeTag::TY_SUM) {
      auto discr = measure.SumDiscriminant(underlying, field);
      fmt::print("  storew {}, {}\n", discr, target_id_.Emit());
    }

    auto target = parent_.GenTemporary();
    fmt::print("  {} = l copy {}\n", target.Emit(), target_id_.Emit());

    for (auto& i : node->initializers_) {
      auto offset = measure.MeasureFieldOffset(node->GetType(), i.field);

      // Move the pointer
      fmt::print("  {} =l add {}, {}\n", target.Emit(), target.Emit(),
                 offset - previous_offset);

      previous_offset = offset;

      if (i.init) {
        parent_.GenAtAddress(i.init, target);
      }
    }
  }

  virtual void VisitNew(NewExpression* node) override {
    auto mem = parent_.Eval(node);
    fmt::print("  storel {}, {} \n", mem.Emit(), target_id_.Emit());
  }

  virtual void VisitAddressof(AddressofExpression* node) override {
    auto mem = parent_.Eval(node);
    fmt::print("  storel {}, {} \n", mem.Emit(), target_id_.Emit());
  }

  virtual void VisitUnary(UnaryExpression* node) override {
    auto id = parent_.Eval(node);
    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitIf(IfExpression* node) override {
    auto id = parent_.Eval(node);

    if (parent_.measure_.IsCompound(node->GetType())) {
      auto [s, a] = parent_.SizeAlign(node);
      parent_.Copy(a, s, id, target_id_);
      return;
    }

    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitTypecast(TypecastExpression* node) override {
    auto id = parent_.Eval(node);
    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitBinary(BinaryExpression* node) override {
    auto id = parent_.Eval(node);
    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitComparison(ComparisonExpression* node) override {
    auto id = parent_.Eval(node);
    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitBlock(BlockExpression* node) override {
    auto id = parent_.Eval(node);

    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitVarAccess(VarAccessExpression* node) override {
    auto id = parent_.Eval(node);

    if (parent_.measure_.IsCompound(node->GetType())) {
      auto [s, a] = parent_.SizeAlign(node);
      parent_.Copy(a, s, id, target_id_);
      return;
    }

    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitMatch(MatchExpression* node) override {
    auto id = parent_.Eval(node);

    if (parent_.measure_.IsCompound(node->GetType())) {
      auto [s, a] = parent_.SizeAlign(node);
      parent_.Copy(a, s, id, target_id_);
      return;
    }

    if (parent_.measure_.IsZST(node->GetType())) {
      return;
    }

    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

  virtual void VisitLiteral(LiteralExpression* node) override {
    if (node->GetType()->tag == types::TypeTag::TY_UNIT) {
      return;
    }

    auto id = parent_.Eval(node);
    constexpr auto fmt = "  store{} {}, {} \n";
    fmt::print(fmt, StoreSuf(node->GetType()), id.Emit(), target_id_.Emit());
  }

 private:
  IrEmitter& parent_;
  Value target_id_;
};

}  // namespace qbe
