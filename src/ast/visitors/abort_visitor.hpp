#pragma once

#include <ast/visitors/visitor.hpp>
#include <cstdlib>

class AbortVisitor : public Visitor {
 public:
  virtual void VisitYield(YieldStatement*) override {
    std::abort();
  }

  virtual void VisitReturn(ReturnStatement*) override {
    std::abort();
  }

  virtual void VisitTypeDecl(TypeDeclStatement*) override {
    std::abort();
  }

  virtual void VisitVarDecl(VarDeclStatement*) override {
    std::abort();
  }

  virtual void VisitFunDecl(FunDeclStatement*) override {
    std::abort();
  }

  virtual void VisitTraitDecl(TraitDeclaration*) override {
    std::abort();
  }

  // Patterns

  virtual void VisitBindingPat(BindingPattern*) override {
    std::abort();
  }

  virtual void VisitDiscardingPat(DiscardingPattern*) override {
    std::abort();
  }

  virtual void VisitLiteralPat(LiteralPattern*) override {
    std::abort();
  }

  virtual void VisitStructPat(StructPattern*) override {
    std::abort();
  }

  virtual void VisitVariantPat(VariantPattern*) override {
    std::abort();
  }

  virtual void VisitAssignment(AssignmentStatement*) override {
    std::abort();
  }

  virtual void VisitExprStatement(ExprStatement*) override {
    std::abort();
  }

  // Expressions

  virtual void VisitComparison(ComparisonExpression*) override {
    std::abort();
  }

  virtual void VisitBinary(BinaryExpression*) override {
    std::abort();
  }

  virtual void VisitUnary(UnaryExpression*) override {
    std::abort();
  }

  virtual void VisitDeref(DereferenceExpression*) override {
    std::abort();
  }

  virtual void VisitAddressof(AddressofExpression*) override {
    std::abort();
  }

  virtual void VisitIf(IfExpression*) override {
    std::abort();
  }

  virtual void VisitMatch(MatchExpression*) override {
    std::abort();
  }

  virtual void VisitNew(NewExpression*) override {
    std::abort();
  }

  virtual void VisitBlock(BlockExpression*) override {
    std::abort();
  }

  virtual void VisitFnCall(FnCallExpression*) override {
    std::abort();
  }

  virtual void VisitIntrinsic(IntrinsicCall*) override {
    std::abort();
  }

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr*) override {
    std::abort();
  }

  virtual void VisitFieldAccess(FieldAccessExpression*) override {
    std::abort();
  }

  virtual void VisitVarAccess(VarAccessExpression*) override {
    std::abort();
  }

  virtual void VisitLiteral(LiteralExpression*) override {
    std::abort();
  }

  virtual void VisitTypecast(TypecastExpression*) override {
    std::abort();
  }
};

//////////////////////////////////////////////////////////////////////
