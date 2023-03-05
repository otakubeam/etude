#pragma once

#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <ast/declarations.hpp>

#include <utility>
#include <queue>

namespace types::constraints {

using BindingGroup = std::vector<FunDeclaration*>;

class ConstraintSolver {
 public:
  ConstraintSolver();

  void CollectAndSolve();

  using SortedFuns = std::vector<Declaration*>;
  void CollectAndSolve(SortedFuns& binding_group);

  bool Unify(Type* a, Type* b);

 private:
  void SolveBatch();

  void PrintQueue();
  void ReportErrors();

  void Generalize(Type* ty);
  bool UnifyUnderlyingTypes(Type* a, Type* b);

  void ConstrainGenerics();
  bool TrySolveConstraint(Trait i);
  void GeneralizeBindingGroup(BindingGroup& group);

 private:
  std::vector<BindingGroup> binding_groups_;

  std::deque<Trait> work_queue_;
  std::deque<Trait> fill_queue_;

  std::deque<Trait> errors_;
};

}  // namespace types::constraints
