#pragma once

#include <types/trait.hpp>

#include <queue>

namespace types::constraints {

class ConstraintSolver {
 public:
  ConstraintSolver(std::queue<Trait> work) : work_queue{std::move(work)} {
  }

  void Solve() {
    // When do I stop if I have residual constraints?

    fmt::print("Work queue!\n");
    size_t i = 0;

    while (!work_queue.empty()) {
      if (i++ > 500)
        break;

      auto i = work_queue.front();

      fmt::print("{}\n", FormatTrait(i));

      if (i.tag == TraitTags::TYPES_EQ) {
        Unify(i.types_equal.a, i.types_equal.b);
      } else if (i.tag == TraitTags::HAS_FIELD) {
        // Find leader

        auto ctx = i.bound->typing_context_;
        i.bound = FindLeader(i.bound);
        if (!i.bound->typing_context_) {
          i.bound->typing_context_ = ctx;
        }

        //

        if (i.bound->tag == TypeTag::TY_STRUCT) {
          auto pack = i.bound->as_struct.first;
          for (auto& p : pack) {
            if (p.field == i.has_field.field_name) {
              Unify(p.ty, i.has_field.field_type);
            }
          }
        } else if (i.bound->tag == TypeTag::TY_APP) {
          i.bound = ApplyTyconsLazy(i.bound);
          fmt::print("Applied tycons {}\n", FormatType(*i.bound));
          work_queue.push(i);
        } else {
          work_queue.push(i);
        }
      }
      work_queue.pop();
    }

    fmt::print("End! ---------\n");

    // while (work_queue.size()) {
    //   auto it = std::move(work_queue.front());

    //   work_queue.pop();

    //   switch (it.tag) {
    //     case TraitTags::NUM:
    //     case TraitTags::ORD:
    //     case TraitTags::EQ:
    //     case TraitTags::CALLABLE:
    //     default:
    //       break;
    //   }
    // }
  }

 private:
  std::queue<Trait> work_queue;
};

}  // namespace types::constraints
