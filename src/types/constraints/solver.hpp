#pragma once

#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <ast/declarations.hpp>

#include <boost/container/small_vector.hpp>

#include <utility>
#include <queue>

namespace types::constraints {

using BindingGroup = boost::container::small_vector<FunDeclStatement*, 3>;

class ConstraintSolver {
 public:
  ConstraintSolver();

  void Solve();

  void CollectConstraints();

 private:
  void PrintQueue();
  void ReportErrors();

  bool Unify(Type* a, Type* b);
  bool UnifyUnderlyingTypes(Type* a, Type* b);

  void Generalize(Type* ty);
  Type* Instantinate(Type* ty, KnownParams& map);

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
