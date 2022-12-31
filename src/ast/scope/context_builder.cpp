#include <ast/scope/context_builder.hpp>

#include <ast/declarations.hpp>
#include <ast/patterns.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

ContextBuilder::ContextBuilder(Context& unit_context)
    : unit_context_{unit_context}, current_context_{&unit_context} {
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitTypeDecl(TypeDeclStatement* node) {
  if (!node->body_) {
    std::abort();
  }

  current_context_ =
      current_context_->MakeNewScopeLayer(node->GetLocation(), node->GetName());

  // Replicate * `size` times

  auto kind_args = types::MakeKindParamPack(node->parameters_.size());
  auto kind = MakeFunType(std::move(kind_args), &types::builtin_kind);

  // type Ty t = struct { v: Vec(t), }     <<<----   set context for vec
  types::SetTyContext(node->body_, current_context_);

  auto ty = new types::Type{
      .tag = types::TypeTag::TY_CONS,
      .as_generic = types::TyConsType{.name = node->name_,
                                     .param_pack = node->parameters_,
                                     .body = node->body_,
                                     .kind = kind}};

  types::SetTyContext(ty, current_context_);

  // Accessible from parent
  current_context_->parent->bindings.InsertSymbol(Symbol{
      .sym_type = SymbolType::TYPE,
      .name = node->GetName(),
      .as_type = {.type = ty},
      .declared_at = node->GetLocation(),
  });

  for (auto param : node->parameters_) {
    current_context_->bindings.InsertSymbol({
        .sym_type = SymbolType::TYPE,
        .name = param.GetName(),
        .as_type = {.type = &types::builtin_kind},  // Why?
        .declared_at = param.location,
    });
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitVarDecl(VarDeclStatement* node) {
  if (node->value_) {
    node->value_->Accept(this);
  }

  node->layer_ = current_context_;

  auto inst_ty = node->annotation_ = types::HintedOrNew(node->annotation_);

  // e.g. `of Vec(Vec(Int)) static matrix = ...`
  SetTyContext(inst_ty, current_context_);

  current_context_->bindings.InsertSymbol({
      .sym_type = SymbolType::VAR,
      .name = node->GetName(),
      .as_varbind = {.type = inst_ty},
      .declared_at = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitFunDecl(FunDeclStatement* node) {
  node->layer_ = current_context_;

  auto fun_ty = types::HintedOrNew(node->type_);

  // Handle cases where parts of the signature are known
  // e.g. Vec(_) -> Maybe(_)

  SetTyContext(fun_ty, current_context_);

  if (!node->trait_method_) {
    current_context_->bindings.InsertSymbol({
        .sym_type = SymbolType::FUN,
        .name = node->GetName(),
        .as_fn_sym =
            {
                .argnum = node->formals_.size(),
                .type = fun_ty,
                .def = node,
                .attrs = node->attributes,
            },
        .declared_at = node->GetLocation(),
    });
  }

  if (node->body_) {
    auto symbol = current_context_->RetrieveSymbol(node->GetName());
    symbol->as_fn_sym.def = node;

    current_context_ = current_context_->MakeNewScopeLayer(
        node->body_->GetLocation(), node->GetName());

    node->layer_ = current_context_;

    // Bring parameters into the scope (their very special one)

    for (auto param : node->formals_) {
      current_context_->bindings.InsertSymbol({
          .sym_type = SymbolType::VAR,
          .name = param.GetName(),
          .as_varbind = {.type = types::MakeTypeVar(current_context_)},
          .declared_at = param.location,
      });
    }

    {
      auto fn = current_fn_;  // For return
      current_fn_ = node->GetName();

      node->body_->Accept(this);
      current_fn_ = fn;
    }

    PopScopeLayer();
  }
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitTraitDecl(TraitDeclaration* node) {
  current_context_->bindings.InsertSymbol({
      .sym_type = SymbolType::TRAIT,
      .name = node->GetName(),
      .as_trait = {.decl = node},
      .declared_at = node->GetLocation(),
  });

  for (auto decl : node->methods_) {
    current_context_->bindings.InsertSymbol({
        .sym_type = SymbolType::TRAIT_METHOD,
        .name = decl->GetName(),
        .as_fn_sym = {.def = decl},
        .declared_at = decl->GetLocation(),
    });
  }

  current_context_ =
      current_context_->MakeNewScopeLayer(node->GetLocation(), "Trait scope");

  current_context_->bindings.InsertSymbol(Symbol{
      .sym_type    = SymbolType::TYPE,
      .name        = "Self",
      .as_type     = {.type = types::HintedOrNew(nullptr)},
      .declared_at = node->GetLocation(),
  });

  for (auto decl : node->methods_) {
    decl->Accept(this);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitImplDecl(ImplDeclaration* node) {
  auto trait = current_context_->RetrieveSymbol(node->trait_name_);
  auto& impls = trait->as_trait.decl->impls_;

  impls.push_back(node);

  current_context_ =
      current_context_->MakeNewScopeLayer(node->GetLocation(), "Impl scope");

  current_context_->parent->bindings.InsertSymbol(Symbol{
      .sym_type = SymbolType::TYPE,
      .name = "Self",
      .as_type = {.type = node->for_type_},
      .declared_at = node->GetLocation(),
  });

  for (auto methods : node->trait_methods_) {
    methods->trait_method_ = true;
    methods->Accept(this);
  }

  PopScopeLayer();
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitYield(YieldStatement* node) {
  node->yield_value_->Accept(this);
}

void ContextBuilder::VisitReturn(ReturnStatement* node) {
  node->this_fun = current_fn_;
  node->layer_ = current_context_;
  node->return_value_->Accept(this);
}

void ContextBuilder::VisitAssignment(AssignmentStatement* node) {
  node->value_->Accept(this);
  node->target_->Accept(this);
}

void ContextBuilder::VisitExprStatement(ExprStatement* node) {
  node->expr_->Accept(this);
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitBindingPat(BindingPattern* node) {
  node->layer_ = current_context_;
  node->type_ = types::MakeTypeVar(current_context_);

  current_context_->bindings.InsertSymbol({
      .sym_type = SymbolType::VAR,
      .name = node->name_,
      .as_varbind = {.type = node->type_},
      .declared_at = node->GetLocation(),
  });
}

void ContextBuilder::VisitDiscardingPat(DiscardingPattern*){};

void ContextBuilder::VisitLiteralPat(LiteralPattern*) {
  // No-op
}

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

  for (auto& [pat, expr] : node->patterns_) {
    current_context_ =
        current_context_->MakeNewScopeLayer(pat->GetLocation(), "Match scope");

    pat->Accept(this);
    expr->Accept(this);

    PopScopeLayer();
  }
}

void ContextBuilder::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    node->allocation_size_->Accept(this);
  }

  if (node->initial_value_) {
    node->initial_value_->Accept(this);
  }

  // Handle stuff like `new [10] Vec(_)`
  types::SetTyContext(node->underlying_, current_context_);

  node->type_ = types::MakeTypePtr(node->underlying_);
  types::SetTyContext(node->type_, current_context_);
}

void ContextBuilder::VisitBlock(BlockExpression* node) {
  current_context_ =
      current_context_->MakeNewScopeLayer(node->GetLocation(), "Block scope");

  for (auto stmt : node->stmts_) {
    stmt->Accept(this);
  }

  if (node->final_) {
    node->final_->Accept(this);
  }

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
    if (mem.init) {
      mem.init->Accept(this);
    }
  }
}

void ContextBuilder::VisitFieldAccess(FieldAccessExpression* node) {
  node->layer_ = current_context_;
  node->struct_expression_->Accept(this);
}

void ContextBuilder::VisitVarAccess(VarAccessExpression* node) {
  node->layer_ = current_context_;
}

void ContextBuilder::VisitLiteral(LiteralExpression*) {
  // No-op
}

void ContextBuilder::VisitTypecast(TypecastExpression* node) {
  node->expr_->Accept(this);

  // (...) ~> *Vec(Int)        <<<------- Instantiate
  types::SetTyContext(node->type_, current_context_);
}

}  // namespace ast::scope
