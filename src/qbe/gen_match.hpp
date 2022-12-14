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

class GenMatch : public AbortVisitor {
 public:
  GenMatch(IrEmitter& parent, Value target_id, int next_arm, bool literal)
      : parent_{parent},
        target_id_{target_id},
        literal_(literal),
        next_arm_(next_arm) {
  }

  void VisitBindingPat(BindingPattern* node) {
    parent_.named_values_.insert_or_assign(
        node->name_, literal_ ? target_id_.AsPattern() : target_id_);
  }

  void VisitDiscardingPat(DiscardingPattern*) {
    // No-op
  }

  void VisitLiteralPat(LiteralPattern* node) {
    if (node->pat_->GetType()->tag == types::TypeTag::TY_UNIT) {
      return;
    }

    auto out = parent_.GenTemporary();

    auto against = parent_.Eval(node->pat_);

    auto ty = node->pat_->GetType();
    auto eq_type = ToQbeType(ty);
    auto load_suf = LoadSuf(ty);

    auto target = target_id_;

    if (!literal_) {
      auto load = parent_.GenTemporary();
      fmt::print("  {} = {} load{} {}  \n",  //
                 load.Emit(), eq_type, load_suf, target_id_.Emit());
      target = load;
    }

    auto condition = parent_.GenTemporary();

    fmt::print("  {} =w ceq{} {}, {}\n",  //
               condition.Emit(), eq_type, target.Emit(), against.Emit());
    fmt::print("  jnz {}, @match.{}.check.{}, @match.{}\n",  //
               condition.Emit(), next_arm_ - 1, check++, next_arm_);
    fmt::print("@match.{}.check.{}\n", next_arm_ - 1, check - 1);
  }

  // In the form `.some.next n`  <<---  parsed as VariantPattern
  void VisitSingleStructPat(VariantPattern* node) {
    if (auto& inner = node->inner_pat_) {
      auto offset =
          parent_.measure_.MeasureFieldOffset(node->type_, node->name_);

      auto new_addr = parent_.GenTemporary();

      fmt::print("  {} = l add {}, {}  \n",  //
                 new_addr.Emit(), target_id_.Emit(), offset);

      target_id_ = new_addr;

      inner->Accept(this);
    }
  }

  void VisitVariantPat(VariantPattern* node) {
    auto ty = node->GetType();

    if (ty->tag == types::TypeTag::TY_STRUCT) {
      VisitSingleStructPat(node);
      return;
    }

    auto discr_pat =
        parent_.GenConstInt(parent_.measure_.SumDiscriminant(ty, node->name_));

    auto memory = parent_.GenTemporary();
    fmt::print("  {} =w loadsw {}  \n", memory.Emit(), target_id_.Emit());

    auto condition = parent_.GenTemporary();
    fmt::print("  {} =w ceqw {}, {}\n",  //
               condition.Emit(), discr_pat.Emit(), memory.Emit());

    fmt::print("  jnz {}, @match.{}.check.{}, @match.{}\n",  //
               condition.Emit(), next_arm_ - 1, check++, next_arm_);

    fmt::print("@match.{}.check.{}\n", next_arm_ - 1, check - 1);

    if (auto& inner = node->inner_pat_) {
      auto new_addr = parent_.GenTemporary();

      fmt::print("  {} = l add {}, {}  \n",  //
                 new_addr.Emit(), target_id_.Emit(), 4);

      target_id_ = new_addr;

      inner->Accept(this);
    }
  }

 private:
  IrEmitter& parent_;

  Value target_id_;
  bool literal_ = false;  // For those targets that are not held in memory

  int next_arm_ = -1;
  int check = 0;
  std::string result_;
};

}  // namespace qbe
