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
  auto trait_symbol = current_context_->RetrieveSymbol(node->GetName());

  // Insert the trait methods into the global namespace

  auto* first = trait_symbol->as_trait.methods;

  for (auto method = first; method; method = method->next) {
    auto method_symbol = MakeTraitMethodSymbol(method);

    current_context_->InsertSymbol(method_symbol);
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
                                            1, current_context_);
  auto self_type = MakeTySymbol("Self", self_constructor, node->GetLocation());

  current_context_->InsertSymbol(self_type);

  // Eval all methods

  auto* first = impl->methods;

  for (auto method = first; method; method = method->next) {
    Eval(method->definition);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitModuleDecl(ModuleDeclaration* node) {
  auto module = WithItemsSeparated(node);
  module->module_context_ = current_context_;

  // Add module symbol to the global scope

  current_context_->InsertSymbol(MakeModSymbol(node->GetName(),  //
                                               module,           //
                                               node->GetLocation()));

  EnterScopeLayer(node->GetLocation(), "Module scope");

  // Freestanding functions

  for (auto func = module->functions; func; func = func->next) {
    //
    // Build the actual symbol
    //
    // Must do it here in order to differenciate trait methods from functions
    // (methods are added in `VisitTraitDecl`)
    //
    auto InsertFunctionSymbol = [func, this] {
      fmt::print("[!] Proc: {}\n", func->definition->GetName());
      current_context_->InsertSymbol(
          MakeFunSymbol(func->definition->GetName(),  //
                        func,                         //
                        func->definition->GetLocation()));
    };

    InsertFunctionSymbol();
    Eval(func->definition);
  }

  // Freestanding types

  for (auto type = module->types; type; type = type->next) {
    Eval(type->definition);
  }

  for (auto trait = module->traits; trait; trait = trait->next) {
    //
    // Construct the trait symbol by first separating the items
    //    into their categories: methods / types / constants.
    //
    auto trait_symbol = MakeTraitSymbol(trait->me->GetName(),           //
                                        WithItemsSeparated(trait->me),  //
                                        trait->me->GetLocation());

    current_context_->InsertSymbol(trait_symbol);

    Eval(trait->me);
  }

  // Process Impls only after Traits

  for (auto impl = module->impls; impl; impl = impl->next_) {
    Eval(impl);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitTypeDecl(TypeDeclaration* node) {
  EnterScopeLayer(node->GetLocation(), node->GetName());

  // type Ty t = struct { v: Vec(t), }     <<<----   set context for vec

  types::SetTyContext(node->body_, current_context_);

  // Build the kind for the type constructor: e.g. `Vec :: * -> *`

  auto kind = node->parameters_.size();
  auto ty_cons = types::MakeTyCons(node->name_, std::move(node->parameters_),
                                   node->body_, kind, current_context_);

  // Make constructor accessible from parent scope

  current_context_->parent->InsertSymbol(MakeTySymbol(node->GetName(),  //
                                                      ty_cons,          //
                                                      node->GetLocation()));

  // Instantiate parameters in the new scope

  for (auto param : node->parameters_) {
    current_context_->InsertSymbol(MakeTySymbol(param.GetName(),       //
                                                &types::builtin_kind,  //
                                                param.location));
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitVarDecl(VarDeclaration* node) {
  node->layer_ = current_context_;

  MaybeEval(node->value_);

  auto binding_type = node->annotation_ = types::HintedOrNew(node->annotation_);

  SetTyContext(binding_type, current_context_);

  current_context_->InsertSymbol(MakeVarSymbol(node->GetName(),  //
                                               binding_type,     //
                                               node->GetLocation()));
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitFunDecl(FunDeclaration* node) {
  if (!node->body_) {
    return;
  }

  node->layer_ = current_context_;

  auto fun_ty = types::HintedOrNew(node->type_);

  SetTyContext(fun_ty, current_context_);

  // Provide the definition to the symbol table

  auto symbol = current_context_->RetrieveSymbol(node->GetName());

  auto ExchangeBody = [symbol, node]() {
    auto prev = std::exchange(symbol->as_fun->definition->body_, node->body_);
    if (prev && prev != node->body_) {
      throw std::runtime_error{fmt::format(
          "Multiple definitions of a function {}", node->GetName())};
    }
  };

  ExchangeBody();

  EnterScopeLayer(node->body_->GetLocation(), node->GetName());

  // Insert the parameters into the table

  for (auto param : node->formals_) {
    auto ty_variable = types::MakeTypeVar(current_context_);

    auto symbol = MakeVarSymbol(param.GetName(),  //
                                ty_variable,      //
                                param.location);

    current_context_->InsertSymbol(symbol);
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

  current_context_->InsertSymbol(binding_symbol);
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
  Eval(node->value_);
  MaybeEval(node->else_rest_);

  EnterScopeLayer(node->GetLocation(), "Let scope");

  Eval(node->pattern_);  // Add a new binding
  Eval(node->rest_);

  PopScopeLayer();
}

void ContextBuilder::VisitBlock(BlockExpression* node) {
  EnterScopeLayer(node->GetLocation(), "Block scope");

  //
  // TODO: add the declarations
  //

  Eval(node->expr_);

  PopScopeLayer();
}

void ContextBuilder::VisitIndex(IndexExpression* node) {
  Eval(node->indexed_expr_);
  Eval(node->index_);
}

void ContextBuilder::VisitFnCall(FnCallExpression* node) {
  node->layer_ = current_context_;

  Eval(node->callable_);

  for (auto& a : node->arguments_) {
    a->Accept(this);
  }
}

void ContextBuilder::VisitIntrinsic(IntrinsicCall* node) {
  VisitFnCall(node);
}

void ContextBuilder::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  for (auto mem : node->initializers_) {
    MaybeEval(mem.init);
  }
}

void ContextBuilder::VisitFieldAccess(FieldAccessExpression* node) {
  node->layer_ = current_context_;
  node->struct_expression_->Accept(this);

  if (node->layer_ == current_context_) {
    return;
  }

  // This was a namespace

  auto field = node->field_name_.GetName();

  // Is field a namespace too?
  // Retrieve from current context

  if (auto mod_ctx = TryEnterModuleCtx(field)) {
    node->layer_ = current_context_ = mod_ctx;
  } else {
    //
    // If it's not a namespace then it's the final name
    // Return current_context_ to its previous value
    //
    // TODO: make name resolution a separate pass
    //
    current_context_ = node->layer_;
  }
}

void ContextBuilder::VisitVarAccess(VarAccessExpression* node) {
  // Modules are updated eagerly

  if (auto mod_ctx = TryEnterModuleCtx(node->GetName())) {
    node->layer_ = current_context_ = mod_ctx;
  }

  // While ordinary symbol are updated at once

  node->layer_ = current_context_;
}

void ContextBuilder::VisitLiteral(LiteralExpression*){};

void ContextBuilder::VisitTypecast(TypecastExpression* node) {
  node->expr_->Accept(this);
  types::SetTyContext(node->type_, current_context_);
}

}  // namespace ast::scope
