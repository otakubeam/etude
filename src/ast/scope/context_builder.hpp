#pragma once

#include <ast/visitors/return_visitor.hpp>

#include <ast/scope/context.hpp>

#include <vector>

namespace ast::scope {

class ContextBuilder : public ReturnVisitor<int> {
 public:
  ContextBuilder(Context& unit_context);

  void VisitVarDecl(VarDeclaration* node) override;
  void VisitFunDecl(FunDeclaration* node) override;
  void VisitTypeDecl(TypeDeclaration* node) override;
  void VisitImplDecl(ImplDeclaration* node) override;
  void VisitTraitDecl(TraitDeclaration* node) override;
  void VisitModuleDecl(ModuleDeclaration* node) override;

  void VisitDiscardingPat(DiscardingPattern* node) override;
  void VisitBindingPat(BindingPattern* node) override;
  void VisitLiteralPat(LiteralPattern* node) override;
  void VisitVariantPat(VariantPattern* node) override;

  void VisitAssign(AssignExpression* node) override;
  void VisitSeqExpr(SeqExpression* node) override;
  void VisitComparison(ComparisonExpression* node) override;
  void VisitBinary(BinaryExpression* node) override;
  void VisitUnary(UnaryExpression* node) override;
  void VisitIf(IfExpression* node) override;
  void VisitNew(NewExpression* node) override;
  void VisitLet(LetExpression* node) override;
  void VisitMatch(MatchExpression* node) override;
  void VisitYield(YieldExpression* node) override;
  void VisitBlock(BlockExpression* node) override;
  void VisitReturn(ReturnExpression* node) override;
  void VisitFnCall(FnCallExpression* node) override;
  void VisitIntrinsic(IntrinsicCall* node) override;
  void VisitLiteral(LiteralExpression* node) override;
  void VisitTypecast(TypecastExpression* node) override;
  void VisitDeref(DereferenceExpression* node) override;
  void VisitAddressof(AddressofExpression* node) override;
  void VisitVarAccess(VarAccessExpression* node) override;
  void VisitFieldAccess(FieldAccessExpression* node) override;
  void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  void EnterScopeLayer(lex::Location loc, std::string_view name);
  void PopScopeLayer();

  void MaybeEval(TreeNode* node);

  using I = ImplSymbol;
  using T = TraitSymbol;
  using M = ModuleSymbol;
  auto WithItemsSeparated(ImplDeclaration* node) -> I*;
  auto WithItemsSeparated(TraitDeclaration* node) -> T;
  auto WithItemsSeparated(ModuleDeclaration* node) -> M*;

 public:
  // For debug dumping all symbols in the program
  std::vector<Context*> debug_context_leafs_{current_context_};

 private:
  Context& unit_context_;
  Context* current_context_{&unit_context_};

  std::string_view current_fn_;
};

}  // namespace ast::scope
