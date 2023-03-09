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

  void CollectAndSolve(ast::scope::ModuleSymbol* mod);

  bool Unify(Type* a, Type* b);

 private:
  void SolveBatch();

  void PrintQueue();
  void ReportErrors();

  bool UnifyUnderlyingTypes(Type* left, Type* right);

  bool CompareParameterLists(Parameter* a, Parameter* b);
  bool UnifyStructs(StructTy* a, StructTy* b);
  bool UnifyTyApp(TyAppType* a, TyAppType* b);
  bool UnifyFunc(FunType* a, FunType* b);
  bool UnifyPtr(PtrType* a, PtrType* b);

  void Generalize(Type* ty);

  void ConstrainGenerics();
  bool TrySolveConstraint(Trait constraint);
  void GeneralizeBindingGroup(BindingGroup& group);

 private:
  std::vector<BindingGroup> binding_groups_;

  std::deque<Trait> work_queue_;
  std::deque<Trait> fill_queue_;

  std::deque<Trait> errors_;
};

}  // namespace types::constraints
