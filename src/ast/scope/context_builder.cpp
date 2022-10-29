#include <ast/scope/context_builder.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitTypeDecl(TypeDeclStatement* node) {
  if (!node->body_) {
    std::abort();
  }

  current_context_ = current_context_->MakeNewScopeLayer(node->GetLocation(),
                                                         node->GetStructName());

  // Replicate * `size` times

  auto kind_args = types::MakeKindParamPack(node->parameters_.size());
  auto kind = types::MakeFunType(std::move(kind_args), &types::builtin_kind);

  auto ty = new types::Type{.tag = types::TypeTag::TY_CONS,
                            .as_tycons = types::TyConsType{
                                .name = node->name_,
                                .param_pack = node->parameters_,
                                .body = node->body_,
                                .kind = kind,
                            }};

  // Accessible from parent

  current_context_->parent->bindings.InsertSymbol(Symbol{
      .sym_type = SymbolType::TYPE,
      .is_complete = node->body_ != nullptr,
      .name = node->GetStructName(),
      // .as_type = {.kind = kind, .body = ty, .layer = current_context_},
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

  // e.g. `of Vec(Vec(Int)) static matrix = ...`

  auto inst_ty =
      ConstructType(types::HintedOrNew(node->annotation_), current_context_);

  current_context_->bindings.InsertSymbol({
      .sym_type = SymbolType::VAR,
      .is_complete = node->value_ != nullptr,
      .name = node->GetVarName(),
      .as_varbind = {.type = inst_ty},
      .declared_at = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitFunDecl(FunDeclStatement* node) {
  node->layer_ = current_context_;

  // Handle cases where pars of the signature are known
  // e.g. Vec(_) -> Maybe(_)

  auto inst_fun_ty =
      ConstructType(types::HintedOrNew(node->type_), current_context_);

  current_context_->bindings.InsertSymbol({
      .sym_type = SymbolType::FUN,
      .is_complete = node->body_ != nullptr,
      .name = node->GetFunctionName(),
      .as_fn_sym = {.type = inst_fun_ty},
      .declared_at = node->GetLocation(),
  });

  if (node->body_) {
    current_context_ = current_context_->MakeNewScopeLayer(
        node->body_->GetLocation(), node->GetFunctionName());

    node->layer_ = current_context_;

    // Bring parameters into the scope (their very special one)

    for (auto param : node->formals_) {
      current_context_->bindings.InsertSymbol({
          .sym_type = SymbolType::VAR,
          .is_complete = true,
          .name = param.GetName(),
          .as_varbind = {.type = types::MakeTypeVar()},
          .declared_at = param.location,
      });
    }

    node->body_->Accept(this);

    PopScopeLayer();
  }
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitYield(YieldStatement* node) {
  node->yield_value_->Accept(this);
}

void ContextBuilder::VisitReturn(ReturnStatement* node) {
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
  node->operand_->Accept(this);
}

void ContextBuilder::VisitAddressof(AddressofExpression* node) {
  node->operand_->Accept(this);
}

void ContextBuilder::VisitIf(IfExpression* node) {
  node->condition_->Accept(this);
  node->true_branch_->Accept(this);
  node->false_branch_->Accept(this);
}

void ContextBuilder::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    node->allocation_size_->Accept(this);
  }

  // Handle stuff like `new [10] Vec(_)`
  auto inst = ConstructType(node->underlying_, current_context_);

  node->type_ = types::MakeTypePtr(inst);
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
  for (auto val : node->values_) {
    val->Accept(this);
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

  // (...) ~> *Vec(Int)        <<<------- instantiate
  node->type_ = ConstructType(node->type_, current_context_);
}

}  // namespace ast::scope
