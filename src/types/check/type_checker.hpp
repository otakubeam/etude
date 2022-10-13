#pragma once

#include <types/type.hpp>

#include <ast/visitors/template_visitor.hpp>
#include <ast/scope/environment.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

class TypeChecker : public ReturnVisitor<Type*> {
 public:
  TypeChecker();
  virtual ~TypeChecker();

  virtual void VisitAssignment(AssignmentStatement*) override;
  virtual void VisitVarDecl(VarDeclStatement*) override;
  virtual void VisitStructDecl(StructDeclStatement*) override;
  virtual void VisitFunDecl(FunDeclStatement*) override;
  virtual void VisitReturn(ReturnStatement*) override;
  virtual void VisitYield(YieldStatement*) override;
  virtual void VisitExprStatement(ExprStatement*) override;

  virtual void VisitDeref(DereferenceExpression*) override;
  virtual void VisitAddressof(AddressofExpression*) override;
  virtual void VisitComparison(ComparisonExpression*) override;
  virtual void VisitBinary(BinaryExpression*) override;
  virtual void VisitUnary(UnaryExpression*) override;
  virtual void VisitIf(IfExpression*) override;
  virtual void VisitNew(NewExpression*) override;
  virtual void VisitBlock(BlockExpression*) override;
  virtual void VisitFnCall(FnCallExpression*) override;
  virtual void VisitStructConstruction(StructConstructionExpression*) override;
  virtual void VisitFieldAccess(FieldAccessExpression*) override;
  virtual void VisitLiteral(LiteralExpression*) override;
  virtual void VisitVarAccess(VarAccessExpression*) override;

 private:
  Type* fn_return_expect = nullptr;

  ////////////////////////////////////////////////////////////////////

  using TypeStore = Environment<Type*>;
  TypeStore global_type_store = TypeStore::MakeGlobal();

  TypeStore* variable_type_store_ = &global_type_store;

  ////////////////////////////////////////////////////////////////////

  using DeclStore = Environment<StructDeclStatement*>;
  DeclStore struct_decls_ = DeclStore::MakeGlobal();
};

//////////////////////////////////////////////////////////////////////

}  // namespace types::check
