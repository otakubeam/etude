#pragma once

#include <types/type.hpp>

#include <ast/visitors/template_visitor.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

class TypeChecker : public ReturnVisitor<Type*> {
 public:
  TypeChecker();
  virtual ~TypeChecker();

  virtual void VisitStatement(Statement*) override;

  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitVarDecl(VarDeclStatement* var_decl) override;
  virtual void VisitStructDecl(StructDeclStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* fn_decl) override;
  virtual void VisitReturn(ReturnStatement* return_stmt) override;
  virtual void VisitYield(YieldStatement*) override;
  virtual void VisitExprStatement(ExprStatement* expr_stmt) override;

  virtual void VisitExpression(Expression*) override;

  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitComparison(ComparisonExpression* cmp_expr) override;
  virtual void VisitBinary(BinaryExpression* bin_expr) override;
  virtual void VisitUnary(UnaryExpression* un_expr) override;
  virtual void VisitIf(IfExpression* if_expr) override;
  virtual void VisitBlock(BlockExpression* block) override;
  virtual void VisitFnCall(FnCallExpression* fn_call) override;
  virtual void VisitStructConstruction(StructConstructionExpression*) override;
  virtual void VisitFieldAccess(FieldAccessExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* lit) override;
  virtual void VisitVarAccess(VarAccessExpression* ident) override;

 private:
  Type* fn_return_expect = nullptr;

  ////////////////////////////////////////////////////////////////////

  using TypeStore = Environment<Type*>;
  TypeStore global_type_store = TypeStore::MakeGlobal();

  TypeStore* env_ = &global_type_store;

  ////////////////////////////////////////////////////////////////////

  using DeclStore = Environment<StructDeclStatement*>;
  DeclStore struct_decls_ = DeclStore::MakeGlobal();
};

//////////////////////////////////////////////////////////////////////

}  // namespace types::check
