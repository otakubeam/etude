#pragma once

#include <ast/visitors/return_visitor.hpp>
#include <ast/scope/context.hpp>

#include <types/constraints/solver.hpp>
#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <queue>

namespace types::constraints {

class ConstraintGenerator : public ReturnVisitor<Type*> {
 public:
  ConstraintGenerator(std::deque<Trait>& work_queue, ConstraintSolver& solver)
      : solver_{solver}, work_queue_{work_queue} {
  }

  void VisitVarDecl(VarDeclaration* node) override;
  void VisitFunDecl(FunDeclaration* node) override;
  void VisitTypeDecl(TypeDeclaration* node) override;
  void VisitImplDecl(ImplDeclaration* node) override;
  void VisitTraitDecl(TraitDeclaration* node) override;
  void VisitModuleDecl(ModuleDeclaration* node) override;

  void VisitDiscardingPat(DiscardingPattern* node) override;
  void VisitBindingPat(BindingPattern* node) override;
  void VisitLiteralPat(LiteralPattern* node) override;
  void VisitVariantPat(VariantPattern* node) override;

  void VisitAssign(AssignExpression* node) override;
  void VisitSeqExpr(SeqExpression* node) override;
  void VisitComparison(ComparisonExpression* node) override;
  void VisitBinary(BinaryExpression* node) override;
  void VisitUnary(UnaryExpression* node) override;
  void VisitIf(IfExpression* node) override;
  void VisitNew(NewExpression* node) override;
  void VisitLet(LetExpression* node) override;
  void VisitMatch(MatchExpression* node) override;
  void VisitYield(YieldExpression* node) override;
  void VisitBlock(BlockExpression* node) override;
  void VisitIndex(IndexExpression* node) override;
  void VisitReturn(ReturnExpression* node) override;
  void VisitFnCall(FnCallExpression* node) override;
  void VisitIntrinsic(IntrinsicCall* node) override;
  void VisitLiteral(LiteralExpression* node) override;
  void VisitTypecast(TypecastExpression* node) override;
  void VisitDeref(DereferenceExpression* node) override;
  void VisitAddressof(AddressofExpression* node) override;
  void VisitVarAccess(VarAccessExpression* node) override;
  void VisitFieldAccess(FieldAccessExpression* node) override;
  void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  void PushEqual(lex::Location loc, Type* a, Type* b);

 private:
  ConstraintSolver& solver_;

  std::deque<Trait>& work_queue_;

  std::string_view current_function_;
};

}  // namespace types::constraints
