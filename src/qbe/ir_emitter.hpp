#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/measure.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <unordered_map>
#include <utility>

namespace qbe {

class GenAddr;
class GenAt;

class IrEmitter : public ReturnVisitor<Value> {
 public:
  friend class GenAddr;
  friend class GenAt;

  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;
  virtual void VisitTypeDecl(TypeDeclStatement* node) override;
  virtual void VisitReturn(ReturnStatement* node) override;
  virtual void VisitYield(YieldStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitNew(NewExpression* node) override;
  virtual void VisitBlock(BlockExpression* node) override;
  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression*) override;
  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitIntrinsic(IntrinsicCall* node) override;
  virtual void VisitFieldAccess(FieldAccessExpression* node) override;
  virtual void VisitVarAccess(VarAccessExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* node) override;
  virtual void VisitTypecast(TypecastExpression* node) override;
  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  uint8_t GetTypeSize(types::Type* t) {
    return measure_.MeasureSize(t);
  }

  Value GenTemporary() {
    return {.tag = Value::TEMPORARY, .id = id_ += 1};
  }

  Value GenConstInt(int value) {
    return {.tag = Value::CONST_INT, .value = value};
  }

  Value GenGlobal(std::string_view name) {
    return {.tag = Value::GLOBAL, .name = name};
  }

 private:
  int id_ = 0;
  std::unordered_map<std::string_view, Value> named_values_;

  detail::SizeMeasure measure_;
};

}  // namespace qbe
