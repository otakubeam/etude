#pragma once

#include <ast/visitors/abort_visitor.hpp>
#include <ast/scope/context.hpp>

#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <queue>

namespace ast::elaboration {

class QuantifyTypes : public AbortVisitor {
 public:
  QuantifyTypes() {
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
};

}  // namespace ast::elaboration
