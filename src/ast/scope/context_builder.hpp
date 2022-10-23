#pragma once

#include <ast/scope/context.hpp>

#include <ast/visitors/visitor.hpp>

namespace ast::scope {

class ContextBuilder : public Visitor {
 public:
  ContextBuilder(Context& unit_context)
      : unit_context_{unit_context}, current_context_{&unit_context} {
  }

  virtual void VisitYield(YieldStatement* node) override;
  virtual void VisitReturn(ReturnStatement* node) override;
  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitTypeDecl(TypeDeclStatement* node) override;
  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;

  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression* node) override;
  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
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
    // debug_context_leafs_.push_back(current_context_);
    current_context_->Print();
    current_context_ = current_context_->parent;
  }

 private:
  Context& unit_context_;
  Context* current_context_;

 public:
  // For dumping all symbols in the program
  std::vector<Context*> debug_context_leafs_;
};

}  // namespace ast::scope
