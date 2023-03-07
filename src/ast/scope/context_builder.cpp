#include <ast/scope/context_builder.hpp>

#include <ast/declarations.hpp>
#include <ast/patterns.hpp>

#include <utility>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

ContextBuilder::ContextBuilder(Context& unit_context)
    : unit_context_{unit_context}, current_context_{&unit_context} {
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitTraitDecl(TraitDeclaration* node) {
  // Construct the trait symbol by first separating the items
  //    into their categories: methods / types / constants.

  auto trait_symbol = MakeTraitSymbol(node->GetName(),           //
                                      WithItemsSeparated(node),  //
                                      node->GetLocation());

  current_context_->bindings.InsertSymbol(trait_symbol);

  // Insert the trait methods into the global namespace

  auto* first = trait_symbol.as_trait.methods;

  for (auto method = first; method; method = method->next) {
    auto method_symbol = MakeTraitMethodSymbol(method);
    current_context_->bindings.InsertSymbol(method_symbol);
  }

  EnterScopeLayer(node->GetLocation(), "Trait scope");

  //
  // TODO: Define Self and the generic types of this trait
  //

  for (auto method = first; method; method = method->next) {
    MaybeEval(method->blanket);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitImplDecl(ImplDeclaration* node) {
  auto trait = current_context_->RetrieveSymbol(node->trait_name_);

  auto impl = WithItemsSeparated(node);
  impl->next = std::exchange(trait->as_trait.impls, impl);

  types::SetTyContext(node->for_type_, current_context_);

  EnterScopeLayer(node->GetLocation(), "Impl scope");

  // Insert Self into the context

  auto self_constructor = types::MakeTyCons("Self", {}, node->for_type_,  //
                                            nullptr, current_context_);
  auto self_type = MakeTySymbol("Self", self_constructor, node->GetLocation());

  current_context_->bindings.InsertSymbol(self_type);

  // Eval all methods

  auto* first = impl->methods;
  for (auto method = first; method; method = method->next) {
    Eval(method->definition);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitTypeDecl(TypeDeclaration* node) {
  EnterScopeLayer(node->GetLocation(), node->GetName());

  // type Ty t = struct { v: Vec(t), }     <<<----   set context for vec

  types::SetTyContext(node->body_, current_context_);

  // Build the kind for the type constructor: e.g. `Vec :: * -> *`
  auto kind_args = types::MakeKindParamPack(node->parameters_.size());
  auto kind = MakeFunType(std::move(kind_args), &types::builtin_kind);

  auto ty_cons = types::MakeTyCons(node->name_, std::move(node->parameters_),
                                   node->body_, kind, current_context_);

  // Make constructor accessible from parent scope

  auto ty_symbol = MakeTySymbol(node->GetName(), ty_cons, node->GetLocation());
  current_context_->parent->bindings.InsertSymbol(std::move(ty_symbol));

  // Instantiate parameters in the new scope

  for (auto param : node->parameters_) {
    auto name = param.GetName();
    auto kind = &types::builtin_kind;
    auto ty_param = MakeTySymbol(name, kind, param.location);
    current_context_->bindings.InsertSymbol(ty_param);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitVarDecl(VarDeclaration* node) {
  node->layer_ = current_context_;

  MaybeEval(node->value_);

  auto binding_type = node->annotation_ = types::HintedOrNew(node->annotation_);

  SetTyContext(binding_type, current_context_);

  auto name = node->GetName();
  auto location = node->GetLocation();
  auto binding_symbol = MakeVarSymbol(name, binding_type, location);

  current_context_->bindings.InsertSymbol(binding_symbol);
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitFunDecl(FunDeclaration* node) {
  if (!node->body_) {
    return;
  }

  node->layer_ = current_context_;

  auto fun_ty = types::HintedOrNew(node->type_);

  SetTyContext(fun_ty, current_context_);

  //
  // Note: This should happen where context is built
  //
  // auto fun_symbol = MakeFunSymbol(node);
  // current_context_->bindings.InsertSymbol();
  //

  // Provide the definition to the symbol table

  auto symbol = current_context_->RetrieveSymbol(node->GetName());

  if (std::exchange(symbol->as_fun.definition->body_, node->body_)) {
    throw std::runtime_error{"Multiple definitions of a function"};
  }

  EnterScopeLayer(node->body_->GetLocation(), node->GetName());

  // Insert the parameters into the table

  for (auto param : node->formals_) {
    auto ty_variable = types::MakeTypeVar(current_context_);

    auto symbol = MakeVarSymbol(param.GetName(),  //
                                ty_variable,      //
                                param.location);

    current_context_->bindings.InsertSymbol(symbol);
  }

  // Build the body of the function

  node->body_->Accept(this);

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitYield(YieldExpression* node) {
  node->yield_value_->Accept(this);
}

void ContextBuilder::VisitReturn(ReturnExpression* node) {
  node->layer_ = current_context_;
  node->return_value_->Accept(this);
}

void ContextBuilder::VisitAssign(AssignExpression* node) {
  node->value_->Accept(this);
  node->target_->Accept(this);
}

void ContextBuilder::VisitSeqExpr(SeqExpression* node) {
  node->expr_->Accept(this);
  MaybeEval(node->rest_);
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitBindingPat(BindingPattern* node) {
  node->layer_ = current_context_;
  node->type_ = types::MakeTypeVar(current_context_);

  auto binding_symbol = MakeVarSymbol(node->name_,  //
                                      node->type_,  //
                                      node->GetLocation());

  current_context_->bindings.InsertSymbol(binding_symbol);
}

void ContextBuilder::VisitDiscardingPat(DiscardingPattern*){};
void ContextBuilder::VisitLiteralPat(LiteralPattern*){};

void ContextBuilder::VisitVariantPat(VariantPattern* node) {
  node->layer_ = current_context_;

  if (auto inner = node->inner_pat_) {
    inner->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitComparison(ComparisonExpression* node) {
  node->left_->Accept(this);
  node->right_->Accept(this);
}

void ContextBuilder::VisitBinary(BinaryExpression* node) {
  node->left_->Accept(this);
  node->right_->Accept(this);
}

void ContextBuilder::VisitUnary(UnaryExpression* node) {
  node->operand_->Accept(this);
}

void ContextBuilder::VisitDeref(DereferenceExpression* node) {
  node->layer_ = current_context_;
  node->operand_->Accept(this);
}

void ContextBuilder::VisitAddressof(AddressofExpression* node) {
  node->layer_ = current_context_;
  node->operand_->Accept(this);
}

void ContextBuilder::VisitIf(IfExpression* node) {
  node->condition_->Accept(this);
  node->true_branch_->Accept(this);
  node->false_branch_->Accept(this);
}

void ContextBuilder::VisitMatch(MatchExpression* node) {
  node->against_->Accept(this);

  for (auto& [pattern, expr] : node->patterns_) {
    EnterScopeLayer(pattern->GetLocation(), "Match scope");

    pattern->Accept(this);
    expr->Accept(this);

    PopScopeLayer();
  }
}

void ContextBuilder::VisitNew(NewExpression* node) {
  MaybeEval(node->allocation_size_);
  MaybeEval(node->initial_value_);

  node->type_ = types::MakeTypePtr(node->underlying_);
  types::SetTyContext(node->type_, current_context_);
}

void ContextBuilder::VisitLet(LetExpression* node) {
  std::abort();  // TODO
}

void ContextBuilder::VisitBlock(BlockExpression* node) {
  EnterScopeLayer(node->GetLocation(), "Block scope");

  //
  // TODO: add the declarations
  //

  Eval(node->expr_);

  PopScopeLayer();
}

void ContextBuilder::VisitFnCall(FnCallExpression* node) {
  node->layer_ = current_context_;
  node->callable_->Accept(this);
  for (auto& a : node->arguments_) {
    a->Accept(this);
  }
}

void ContextBuilder::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  for (auto mem : node->initializers_) {
    MaybeEval(mem.init);
  }
}

void ContextBuilder::VisitFieldAccess(FieldAccessExpression* node) {
  node->layer_ = current_context_;
  node->struct_expression_->Accept(this);
}

void ContextBuilder::VisitVarAccess(VarAccessExpression* node) {
  node->layer_ = current_context_;
}

void ContextBuilder::VisitLiteral(LiteralExpression*){};

void ContextBuilder::VisitTypecast(TypecastExpression* node) {
  node->expr_->Accept(this);
  types::SetTyContext(node->type_, current_context_);
}

}  // namespace ast::scope
