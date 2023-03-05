#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>
#include <qbe/measure.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <ast/declarations.hpp>
#include <ast/patterns.hpp>

#include <unordered_map>
#include <utility>

namespace qbe {

class GenMatch;
class GenAddr;
class GenAt;

class IrEmitter : public ReturnVisitor<Value> {
 public:
  friend class GenMatch;
  friend class GenAddr;
  friend class GenAt;

  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitReturn(ReturnExpression* node) override;
  virtual void VisitYield(YieldExpression* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitVarDecl(VarDeclaration* node) override;
  virtual void VisitFunDecl(FunDeclaration* node) override;

  virtual void VisitTraitDecl(TraitDeclaration*) override{};
  virtual void VisitTypeDecl(TypeDeclaration*) override{};
  virtual void VisitImplDecl(ImplDeclaration*) override{};

  virtual void VisitBindingPat(BindingPattern*) override{};
  virtual void VisitLiteralPat(LiteralPattern*) override{};
  virtual void VisitVariantPat(VariantPattern*) override{};

  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitMatch(MatchExpression* node) override;
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

 public:
  void EmitType(types::Type* ty);

  void EmitTypes(std::vector<types::Type*> types);

  ~IrEmitter();

  void EmitStringLiterals();
  void EmitTestArray();

 private:
  auto SizeAlign(types::Type* ty) -> std::pair<uint8_t, size_t>;
  auto SizeAlign(Expression* node) -> std::pair<uint8_t, size_t>;

  char GetStoreSuf(size_t align);
  std::string_view GetLoadSuf(size_t align);
  std::string_view LoadResult(size_t align);

  uint8_t GetTypeSize(types::Type* t);

  void Copy(size_t align, size_t size, Value src, Value dst);

  void CheckAssertion(Expression* cond);
  void CallPrintf(IntrinsicCall* node);
  void CallAbort(Expression* cond);

  Value GenParam();

  Value GenTemporary();

  Value GenConstInt(int value);

  Value GenGlobal(std::string_view name);

  void GenAddress(Expression* what, Value out);

  void GenAtAddress(Expression* what, Value where);

 private:
  int id_ = 0;

  std::unordered_map<std::string_view, Value> named_values_;

  std::vector<std::string_view> string_literals_;
  std::vector<std::string_view> test_functions_;

  std::vector<std::string> error_msg_storage_;

  detail::SizeMeasure measure_;
};

}  // namespace qbe
