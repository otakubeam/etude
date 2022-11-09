#pragma once

#include <types/trait.hpp>
#include <types/type.hpp>

#include <utility>
#include <queue>

namespace types::constraints {

class ConstraintSolver {
 public:
  ConstraintSolver(std::deque<Trait> work) : work_queue{std::move(work)} {
  }

  bool TrySolveConstraint(Trait i) {
    fmt::print("Solving constraint {}\n", FormatTrait(i));
    CheckTypes();

    if (i.tag == TraitTags::TYPES_EQ) {
      Unify(i.types_equal.a, i.types_equal.b, fill_queue);
      return true;
    }

    if (i.tag == TraitTags::ADD) {
      i.bound = FindLeader(i.bound);

      if (i.bound->tag == TypeTag::TY_VARIABLE ||
          i.bound->tag == TypeTag::TY_PARAMETER) {
        i.bound->as_variable.constraints.push_back(i);
      }
      return true;
    }

    if (i.tag == TraitTags::EQ) {
      i.bound = FindLeader(i.bound);
      if (i.bound->tag <= TypeTag::TY_PTR) {
        return true;
      }
    }

    if (i.tag == TraitTags::CONVERTIBLE_TO) {
      i.bound = FindLeader(i.bound);
      i.convertible_to.to_type = FindLeader(i.convertible_to.to_type);

      if (i.convertible_to.to_type->tag == TypeTag::TY_PTR &&
          i.bound->tag == TypeTag::TY_UNIT) {
        // Always convert pointers, no questions asked
        return true;
      }

      if (i.convertible_to.to_type->tag == TypeTag::TY_PTR &&
          i.bound->tag == TypeTag::TY_PTR) {
        // Always convert pointers, no questions asked
        return true;
      }
    }

    if (i.tag == TraitTags::CALLABLE) {
      i.bound = FindLeader(i.bound);
      if (i.bound->tag == TypeTag::TY_FUN) {
        return true;
      } else {
        fill_queue.push_back(i);
        return false;
      }
    }

    if (i.tag == TraitTags::ORD) {
      i.bound = FindLeader(i.bound);
      if (i.bound->tag == TypeTag::TY_INT) {
        return true;
      } else {
        fill_queue.push_back(i);
        return false;
      }
    }

    if (i.tag == TraitTags::HAS_FIELD) {
      i.bound = FindLeader(i.bound);

      if (i.bound->tag == TypeTag::TY_STRUCT) {
        auto pack = i.bound->as_struct.first;

        for (auto& p : pack) {
          if (p.field == i.has_field.field_name) {
            Unify(p.ty, i.has_field.field_type, fill_queue);
            return true;
          }
        }

        throw std::runtime_error{"No such field"};
      }

      if (i.bound->tag == TypeTag::TY_APP) {
        i.bound = ApplyTyconsLazy(i.bound);
        fmt::print("Applied tycons {}\n", FormatType(*i.bound));
        fill_queue.push_back(i);
        return true;
      }

      if (i.bound->tag != TypeTag::TY_VARIABLE) {
        fmt::print("{}\n", FormatType(*i.bound));
        throw std::runtime_error{"Not a variable"};
      }
    }

    fill_queue.push_back(i);
    return false;
  }

  void Solve() {
    bool once_more = true;

    while (std::exchange(once_more, false)) {
      PrintQueue();
      while (work_queue.size()) {
        auto i = std::move(work_queue.front());
        once_more |= TrySolveConstraint(std::move(i));
        work_queue.pop_front();
      }

      std::swap(work_queue, fill_queue);
    }

    if (work_queue.size()) {
      PrintQueue();
      throw std::runtime_error{"Residual constraints remain!"};
    }
  }

  void PrintQueue() {
    for (auto& i : work_queue) {
      fmt::print("{}\n", FormatTrait(i));
    }
    fmt::print("\n\n");
  }

 private:
  std::deque<Trait> work_queue;
  std::deque<Trait> fill_queue;
};

}  // namespace types::constraints