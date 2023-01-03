#pragma once

#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <ast/visitors/visitor.hpp>
#include <ast/scope/context.hpp>

#include <queue>

namespace types::check {

class ExpandTypeVariables : public Visitor {
 public:
  ExpandTypeVariables() {
  }

  void VisitYield(YieldStatement* node) override;
  void VisitReturn(ReturnStatement* node) override;
  void VisitAssignment(AssignmentStatement* node) override;
  void VisitExprStatement(ExprStatement* node) override;

  void VisitTypeDecl(TypeDeclStatement* node) override;
  void VisitVarDecl(VarDeclStatement* node) override;
  void VisitFunDecl(FunDeclStatement* node) override;
  void VisitTraitDecl(TraitDeclaration* node) override;

  void VisitBindingPat(BindingPattern* node) override;
  void VisitDiscardingPat(DiscardingPattern* node) override;
  void VisitLiteralPat(LiteralPattern* node) override;
  void VisitVariantPat(VariantPattern* node) override;

  void VisitComparison(ComparisonExpression* node) override;
  void VisitBinary(BinaryExpression* node) override;
  void VisitUnary(UnaryExpression* node) override;
  void VisitDeref(DereferenceExpression* node) override;
  void VisitAddressof(AddressofExpression* node) override;
  void VisitIf(IfExpression* node) override;
  void VisitMatch(MatchExpression* node) override;
  void VisitNew(NewExpression* node) override;
  void VisitBlock(BlockExpression* node) override;
  void VisitFnCall(FnCallExpression* node) override;
  void VisitIntrinsic(IntrinsicCall* node) override;
  void VisitFieldAccess(FieldAccessExpression* node) override;
  void VisitTypecast(TypecastExpression* node) override;
  void VisitLiteral(LiteralExpression* node) override;
  void VisitVarAccess(VarAccessExpression* node) override;
  void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;
};

}  // namespace types::check
