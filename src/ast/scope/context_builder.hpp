#pragma once

#include <ast/scope/context.hpp>

#include <ast/visitors/abort_visitor.hpp>

namespace ast::scope {

class ContextBuilder : public AbortVisitor {
 public:
  ContextBuilder(Context& unit_context);

  virtual void VisitYield(YieldExpression* node) override;
  virtual void VisitReturn(ReturnExpression* node) override;
  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitBindingPat(BindingPattern* node) override;
  virtual void VisitDiscardingPat(DiscardingPattern*) override;
  virtual void VisitLiteralPat(LiteralPattern* node) override;
  virtual void VisitVariantPat(VariantPattern* node) override;

  virtual void VisitTypeDecl(TypeDeclaration* node) override;
  virtual void VisitVarDecl(VarDeclaration* node) override;
  virtual void VisitFunDecl(FunDeclaration* node) override;
  virtual void VisitTraitDecl(TraitDeclaration* node) override;
  virtual void VisitImplDecl(ImplDeclaration* node) override;

  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression* node) override;
  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitMatch(MatchExpression* node) override;
  virtual void VisitNew(NewExpression* node) override;
  virtual void VisitBlock(BlockExpression* node) override;
  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitFieldAccess(FieldAccessExpression* node) override;
  virtual void VisitTypecast(TypecastExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* node) override;
  virtual void VisitVarAccess(VarAccessExpression* node) override;
  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  void PopScopeLayer() {
    current_context_ = current_context_->parent;
  }

 private:
  Context& unit_context_;
  Context* current_context_{&unit_context_};

  std::string_view current_fn_;

 public:
  // For dumping all symbols in the program
  std::vector<Context*> debug_context_leafs_{current_context_};
};

}  // namespace ast::scope
