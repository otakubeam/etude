#include <types/constraints/solver.hpp>
#include <types/constraints/expand/expand.hpp>
#include <types/constraints/generate/algorithm_w.hpp>

namespace types::constraints {

ConstraintSolver::ConstraintSolver() {
}

// Wrapper to give already sorted defs
void ConstraintSolver::CollectAndSolve(SortedFuns& definitions) {
  for (auto def : definitions) {
    if (auto fun = def->as<FunDeclaration>()) {
      binding_groups_.push_back({fun});
      continue;
    }

    if (auto impl = def->as<ImplDeclaration>()) {
      for (auto method : impl->trait_methods_) {
        binding_groups_.push_back({method});
      }
      continue;
    }

    if (auto trait = def->as<TraitDeclaration>()) {
      for (auto method : trait->methods_) {
        binding_groups_.push_back({method});
      }
    }
  }
  CollectAndSolve();
}

void ConstraintSolver::CollectAndSolve() {
  generate::AlgorithmW generator(work_queue_, *this);

  for (auto& group : binding_groups_) {
    for (auto def : group) {
      def->Accept(&generator);
    }

    SolveBatch();

    GeneralizeBindingGroup(group);

    ConstrainGenerics();

    if (work_queue_.size()) {
      PrintQueue();
      throw std::runtime_error{"Residual constraints remain!"};
    }
  }

  binding_groups_.clear();
}

void ConstraintSolver::ConstrainGenerics() {
  while (work_queue_.size()) {
    auto& q = work_queue_.front();

    if (q.bound->tag != TypeTag::TY_PARAMETER) {
      return;
    }

    q.bound->as_parameter.constraints.push_back(q);

    work_queue_.pop_front();
  }
}

void ConstraintSolver::GeneralizeBindingGroup(BindingGroup& group) {
  for (auto def : group) {
    Generalize(def->type_);
    fmt::print(stderr, "[[Debug]] {} generalized type {}\n", def->GetName(),
               def->type_->Format());
  }
}

bool ConstraintSolver::TrySolveConstraint(Trait i) {
  fmt::print(stderr, "Solving constraint {}\n", FormatTrait(i));
  CheckTypes();

  switch (i.tag) {
    case TraitTags::TYPES_EQ:
      if (!Unify(i.types_equal.a, i.types_equal.b)) {
        errors_.push_back(i);
      }
      return true;

    case TraitTags::ADD:
      i.bound = FindLeader(i.bound);

      if (i.bound->tag == TypeTag::TY_VARIABLE ||
          i.bound->tag == TypeTag::TY_PARAMETER) {
        i.bound->as_parameter.constraints.push_back(i);
      }

      return true;

    case TraitTags::EQ:
      i.bound = FindLeader(i.bound);
      if (i.bound->tag <= TypeTag::TY_PTR) {
        return true;
        fill_queue_.push_back(i);
      }
      return false;

    case TraitTags::CALLABLE:
      i.bound = FindLeader(i.bound);
      if (i.bound->tag == TypeTag::TY_FUN) {
        return true;
      } else {
        fill_queue_.push_back(i);
        return false;
      }

    case TraitTags::ORD:
      i.bound = FindLeader(i.bound);
      if (i.bound->tag <= TypeTag::TY_CHAR) {
        return true;
      } else {
        fill_queue_.push_back(i);
        return false;
      }

    case TraitTags::CONVERTIBLE_TO:
      i.bound = FindLeader(i.bound);
      i.convertible_to.to_type = FindLeader(i.convertible_to.to_type);

      if (i.convertible_to.to_type->tag == TypeTag::TY_PTR &&
          i.bound->tag == TypeTag::TY_UNIT) {
        // Convert Unit to pointers (resulting in nullptr)
        return true;
      }

      if (i.convertible_to.to_type->tag == TypeTag::TY_BOOL &&
          i.bound->tag == TypeTag::TY_PTR) {
        // Contextual conversion of ptr to bool
        return true;
      }

      if (i.convertible_to.to_type->tag == TypeTag::TY_INT &&
          i.bound->tag == TypeTag::TY_CHAR) {
        // Extension
        return true;
      }

      if (i.convertible_to.to_type->tag == TypeTag::TY_PTR &&
          i.bound->tag == TypeTag::TY_PTR) {
        // Always convert pointers, no questions asked
        return true;
      }
      return false;

    case TraitTags::HAS_FIELD:
      i.bound = FindLeader(i.bound);

      if (i.bound->tag == TypeTag::TY_STRUCT) {
        auto pack = i.bound->as_struct.first;

        for (auto& p : pack) {
          if (p.field == i.has_field.field_name) {
            auto field_type = i.has_field.field_type;
            fill_queue_.push_back(MakeTyEqTrait(p.ty, field_type, i.location));
            return true;
          }
        }

        errors_.push_back(i);

        return true;
      }

      if (i.bound->tag == TypeTag::TY_SUM) {
        auto pack = i.bound->as_sum.first;

        for (auto& p : pack) {
          if (p.field == i.has_field.field_name) {
            auto field_type = i.has_field.field_type;
            fill_queue_.push_back(MakeTyEqTrait(p.ty, field_type, i.location));
            return true;
          }
        }

        errors_.push_back(i);
        return true;
      }

      if (i.bound->tag == TypeTag::TY_APP) {
        i.bound = ApplyTyconsLazy(i.bound);
        fmt::print(stderr, "Applied tycons {}\n", FormatType(*i.bound));
        fill_queue_.push_back(i);
        return true;
      }

      if (i.bound->tag != TypeTag::TY_VARIABLE) {
        errors_.push_back(i);
        return true;
      }

    case TraitTags::USER_DEFINED:
    case TraitTags::NUM:
      break;
  }

  fill_queue_.push_back(i);
  return false;
}

void ConstraintSolver::SolveBatch() {
  PrintQueue();

  bool once_more = true;

  while (std::exchange(once_more, false)) {
    while (work_queue_.size()) {
      auto i = std::move(work_queue_.front());
      once_more |= TrySolveConstraint(std::move(i));
      work_queue_.pop_front();
    }

    std::swap(work_queue_, fill_queue_);
  }

  if (errors_.size()) {
    ReportErrors();
    throw std::runtime_error{"Final unification error"};
  }
}

void ConstraintSolver::ReportErrors() {
  for (auto& error : errors_) {
    fmt::print("Cannot satisfy bound {} arising from {}\n",  //
               FormatTrait(error), error.location.Format());
  }
}

void ConstraintSolver::PrintQueue() {
  for (auto& i : work_queue_) {
    fmt::print(stderr, "{}\n", FormatTrait(i));
  }

  fmt::print(stderr, "\n\n");
}

}  // namespace types::constraints
