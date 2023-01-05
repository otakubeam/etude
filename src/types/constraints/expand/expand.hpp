#pragma once

#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <ast/visitors/abort_visitor.hpp>
#include <ast/scope/context.hpp>

#include <queue>

namespace types::constraints {

class ExpandTypeVariables : public AbortVisitor {
 public:
  ExpandTypeVariables() {
  }

  void VisitYield(YieldStatement* node) override;
  void VisitReturn(ReturnStatement* node) override;
  void VisitAssignment(AssignmentStatement* node) override;
  void VisitExprStatement(ExprStatement* node) override;

  void VisitVarDecl(VarDeclStatement* node) override;
  void VisitFunDecl(FunDeclStatement* node) override;
  void VisitTypeDecl(TypeDeclStatement* node) override;
  void VisitTraitDecl(TraitDeclaration* node) override;
  void VisitImplDecl(ImplDeclaration* node) override;

  void VisitBindingPat(BindingPattern*) override{};
  void VisitLiteralPat(LiteralPattern*) override{};
  void VisitVariantPat(VariantPattern*) override{};
  void VisitDiscardingPat(DiscardingPattern*) override{};

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

}  // namespace types::constraints
