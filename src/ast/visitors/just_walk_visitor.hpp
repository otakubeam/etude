#pragma once

#include <ast/visitors/visitor.hpp>
#include <cstdlib>

class JustWalk : public Visitor {
 public:
  virtual void VisitYield(YieldStatement*) {
    std::abort();
  }

  virtual void VisitReturn(ReturnStatement*) {
    std::abort();
  }

  virtual void VisitTypeDecl(TypeDeclStatement*) {
    std::abort();
  }

  virtual void VisitVarDecl(VarDeclStatement*) {
    std::abort();
  }

  virtual void VisitFunDecl(FunDeclStatement*) {
    std::abort();
  }

  virtual void VisitAssignment(AssignmentStatement*) {
    std::abort();
  }

  virtual void VisitExprStatement(ExprStatement*) {
    std::abort();
  }

  // Expressions

  virtual void VisitComparison(ComparisonExpression*) {
    std::abort();
  }

  virtual void VisitBinary(BinaryExpression*) {
    std::abort();
  }

  virtual void VisitUnary(UnaryExpression*) {
    std::abort();
  }

  virtual void VisitDeref(DereferenceExpression*) {
    std::abort();
  }

  virtual void VisitAddressof(AddressofExpression*) {
    std::abort();
  }

  virtual void VisitIf(IfExpression*) {
    std::abort();
  }

  virtual void VisitNew(NewExpression*) {
    std::abort();
  }

  virtual void VisitBlock(BlockExpression*) {
    std::abort();
  }

  virtual void VisitFnCall(FnCallExpression*) {
    std::abort();
  }

  virtual void VisitInstrinsic(IntrinsicCall*) {
    std::abort();
  }

  virtual void VisitCompoundInitalizer(CompoundInitializerExpr*) {
    std::abort();
  }

  virtual void VisitFieldAccess(FieldAccessExpression*) {
    std::abort();
  }

  virtual void VisitVarAccess(VarAccessExpression*) {
    std::abort();
  }

  virtual void VisitLiteral(LiteralExpression*) {
    std::abort();
  }

  virtual void VisitTypecast(TypecastExpression*) {
    std::abort();
  }
};

//////////////////////////////////////////////////////////////////////
