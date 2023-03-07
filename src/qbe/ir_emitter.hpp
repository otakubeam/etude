#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>
#include <qbe/measure.hpp>

#include <ast/visitors/return_visitor.hpp>

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
  void VisitIndex(IndexExpression* node) override;

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
