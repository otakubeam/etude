#pragma once

#include <ast/visitors/template_visitor.hpp>
#include <ast/visitors/abort_visitor.hpp>

#include <utility>

namespace ast::elaboration {

class MarkIntrinsics : public ReturnVisitor<TreeNode*> {
 public:
  void VisitTypeDecl(TypeDeclaration* node) override;
  void VisitVarDecl(VarDeclaration* node) override;
  void VisitFunDecl(FunDeclaration* node) override;
  void VisitTraitDecl(TraitDeclaration* node) override;
  void VisitImplDecl(ImplDeclaration* node) override;

  void VisitBindingPat(BindingPattern* node) override;
  void VisitDiscardingPat(DiscardingPattern* node) override;
  void VisitLiteralPat(LiteralPattern* node) override;
  void VisitVariantPat(VariantPattern* node) override;

  void VisitAssign(AssignExpression* node) override;
  void VisitSeqExpr(SeqExpression* node) override;
  void VisitLet(LetExpression* node) override;
  void VisitComparison(ComparisonExpression* node) override;
  void VisitBinary(BinaryExpression* node) override;
  void VisitUnary(UnaryExpression* node) override;
  void VisitDeref(DereferenceExpression* node) override;
  void VisitAddressof(AddressofExpression* node) override;
  void VisitIf(IfExpression* node) override;
  void VisitMatch(MatchExpression* node) override;
  void VisitYield(YieldExpression* node) override;
  void VisitReturn(ReturnExpression* node) override;
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

}  // namespace ast::elaboration
