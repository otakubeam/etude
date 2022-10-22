#include <ast/scope/context_builder.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitStructDecl(StructDeclStatement* node) {
  current_context_->type_tags.InsertSymbol(Symbol{
      .sym_type = SymbolType::TYPE,
      .is_complete = node->type_ != nullptr,
      .name = node->GetStructName(),
      .as_struct = StructSymbol{.type = node->type_},
      .declared_at = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitVarDecl(VarDeclStatement* node) {
  if (node->value_) {
    node->value_->Accept(this);
  }

  current_context_->bindings.InsertSymbol({
      .sym_type = SymbolType::VAR,
      .is_complete = node->value_ != nullptr,
      .name = node->GetVarName(),
      .as_varbind = {.type = node->annotation_},
      .declared_at = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitFunDecl(FunDeclStatement* node) {
  current_context_->functions.InsertSymbol({
      .sym_type = SymbolType::FUN,
      .is_complete = node->expression_ != nullptr,
      .name = node->GetFunctionName(),
      .as_fn_sym = {.type = node->type_},
      .declared_at = node->GetLocation(),
  });

  if (node->expression_) {
    current_context_ = current_context_->MakeNewScopeLayer(
        node->expression_->GetLocation(), node->GetFunctionName());

    // Bring parameters into the scope (one only for them)

    for (auto param : node->formals_) {
      current_context_->bindings.InsertSymbol({
          .sym_type = SymbolType::VAR,
          .is_complete = true,
          .name = param.GetParameterName(),
          .as_varbind = {.type = param.type},
          .declared_at = node->GetLocation(),
      });
    }

    // If the expression is a block it then opens another scope

    node->expression_->Accept(this);

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
}

void ContextBuilder::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    node->allocation_size_->Accept(this);
  }
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
  node->layer_ = &current_context_->functions;
}

void ContextBuilder::VisitStructConstruction(
    CompoundInitializerExpr* node) {
  for (auto val : node->values_) {
    val->Accept(this);
  }
}

void ContextBuilder::VisitFieldAccess(FieldAccessExpression* node) {
  // I will know the type of struct later, after typecheck
  //
  // dynamic_cast<types::StructType*>(node->GetType())->GetName();
  //
  node->layer_ = &current_context_->type_tags;
}

void ContextBuilder::VisitVarAccess(VarAccessExpression* node) {
  node->layer_ = &current_context_->bindings;
}

void ContextBuilder::VisitLiteral(LiteralExpression*) {
  // No-op
}

void ContextBuilder::VisitTypecast(TypecastExpression* node) {
  node->expr_->Accept(this);
}

}  // namespace ast::scope
