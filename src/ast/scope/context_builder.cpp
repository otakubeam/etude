#pragma once

#include <ast/scope/context_builder.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitStructDecl(StructDeclStatement* node) {
  current_context_->type_tags.InsertSymbol(Symbol{
      .type = SymbolType::TYPE,
      .is_complete = node->type_ != nullptr,
      .name = node->GetStructName(),
      .as = {.struct_symbol = StructSymbol{.type = node->type_}},
      .declared_at = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitVarDecl(VarDeclStatement* node) {
  if (node->value_) {
    node->value_->Accept(this);
  }

  current_context_->bindings.InsertSymbol({
      .type = SymbolType::VAR,
      .is_complete = node->value_ != nullptr,
      .name = node->GetVarName(),
      .as = {.varbind_symbol = {.type = node->annotation_}},
      .declared_at = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void ContextBuilder::VisitFunDecl(FunDeclStatement* node) {
  current_context_->functions.InsertSymbol({
      .type = SymbolType::FUN,
      .is_complete = node->block_ != nullptr,
      .name = node->GetFunctionName(),
      .as = {.fn_symbol = {.type = node->type_}},
      .declared_at = node->GetLocation(),
  });

  // Past insertion for recursion
  if (node->block_) {
    current_context_ = current_context_->MakeNewScopeLayer();
    //
    // Bring parameters into the scope
    //
    for (auto param : node->formals_) {
      current_context_->functions.InsertSymbol({
          .type = SymbolType::VAR,
          .is_complete = true,
          .name = param.GetParameterName(),
          .declared_at = node->GetLocation(),
      });
    }

    node->block_->Accept(this);
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
  node->allocation_size_->Accept(this);
}

void ContextBuilder::VisitBlock(BlockExpression* node) {
  current_context_ = current_context_->MakeNewScopeLayer();

  for (auto stmt : node->stmts_) {
    stmt->Accept(this);
  }

  if (node->final_) {
    node->final_->Accept(this);
  }

  current_context_ = current_context_->parent;
}

void ContextBuilder::VisitFnCall(FnCallExpression* node) {
  node->layer_ = &current_context_->functions;
}

void ContextBuilder::VisitStructConstruction(
    StructConstructionExpression* node) {
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

}  // namespace ast::scope
