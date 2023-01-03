#pragma once

#include <types/constraints/trait.hpp>
#include <types/type.hpp>

#include <ast/visitors/template_visitor.hpp>
#include <ast/scope/context.hpp>
#include <ast/declarations.hpp>

#include <queue>

namespace types::instantiate {

class TemplateInstantiator : public ReturnVisitor<TreeNode*> {
 public:
  TemplateInstantiator(Declaration* main);

  TemplateInstantiator(std::vector<FunDeclStatement*>& tests);

  auto Flush() -> std::pair<std::vector<FunDeclStatement*>, std::vector<Type*>>;

  // Visitor methods

  void VisitYield(YieldStatement* node) override;
  void VisitReturn(ReturnStatement* node) override;
  void VisitAssignment(AssignmentStatement* node) override;
  void VisitExprStatement(ExprStatement* node) override;

  void VisitTypeDecl(TypeDeclStatement* node) override;
  void VisitVarDecl(VarDeclStatement* node) override;
  void VisitFunDecl(FunDeclStatement* node) override;
  void VisitTraitDecl(TraitDeclaration* node) override;
  void VisitImplDecl(ImplDeclaration* node) override;

  void VisitBindingPat(BindingPattern* node) override;
  void VisitDiscardingPat(DiscardingPattern* node) override;
  void VisitLiteralPat(LiteralPattern* node) override;
  void VisitVariantPat(VariantPattern* node) override;

  void VisitComparison(ComparisonExpression* node) override;
  void VisitBinary(BinaryExpression* node) override;
  void VisitUnary(UnaryExpression* node) override;
  void VisitDeref(DereferenceExpression* node) override;
  void VisitAddressof(AddressofExpression* node) override;
  void VisitIf(IfExpression* node) override;
  void VisitMatch(MatchExpression* node) override;
  void VisitNew(NewExpression* node) override;
  void VisitBlock(BlockExpression* node) override;
  void VisitFnCall(FnCallExpression* node) override;
  void VisitIntrinsic(IntrinsicCall* node) override;
  void VisitFieldAccess(FieldAccessExpression* node) override;
  void VisitTypecast(TypecastExpression* node) override;
  void VisitLiteral(LiteralExpression* node) override;
  void VisitVarAccess(VarAccessExpression* node) override;
  void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  using Substitiution = std::unordered_map<Type*, Type*>;

  bool BuildSubstitution(Type* poly, Type* mono, Substitiution& subs);

  auto FindTraitMethod(auto symbol, Type* mono) -> FunDeclStatement*;

  auto GetFunctionDef(auto symbol, Type* mono) -> FunDeclStatement*;

  bool TryFindInstantiation(FnCallExpression* i);

  void ProcessQueueItem(FnCallExpression* i);

  void StartUp(FunDeclStatement* main);

  void MaybeSaveForIL(Type* ty);

  void ProcessQueue();

 private:
  std::deque<FnCallExpression*> instantiation_quque_;
  std::deque<VarAccessExpression*> function_ptrs_;

  ast::scope::Context* call_context_ = nullptr;

  Substitiution current_substitution_;

  std::vector<Type*> types_to_gen_;

  // How do I prevent myself from instantiating something twice or more?
  // A: place instantiated in map: name: string_view -> [](type, fun)

  std::unordered_multimap<std::string_view, FunDeclStatement*> mono_items_;
};

}  // namespace types::instantiate
