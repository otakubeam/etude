#include <ast/elaboration/mark_intrinsics.hpp>

#include <ast/elaboration/intrinsics.hpp>

#include <ast/declarations.hpp>

#include <lex/token.hpp>

namespace ast::elaboration {

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitTypeDecl(TypeDeclaration* node) {
  return_value = node;
}

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitVarDecl(VarDeclaration* node) {
  node->value_ = Eval(node->value_)->as<Expression>();
  return_value = node;
}

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitFunDecl(FunDeclaration* node) {
  if (node->body_) {
    node->body_ = Eval(node->body_)->as<Expression>();
  }
  return_value = node;
}

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitTraitDecl(TraitDeclaration* node) {
  for (auto& method : node->assoc_items_) {
    method = Eval(method)->as<Declaration>();
  }

  return_value = node;
}

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitImplDecl(ImplDeclaration* node) {
  for (auto& method : node->assoc_items_) {
    method = Eval(method)->as<Declaration>();
  }

  return_value = node;
}

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitModuleDecl(ModuleDeclaration* node) {
  for (auto& local : node->local_) {
    local = Eval(local)->as<Declaration>();
  }

  for (auto& exports : node->exported_) {
    exports = Eval(exports)->as<Declaration>();
  }

  return_value = node;
}

//////////////////////////////////////////////////////////////////////

// No-op
void MarkIntrinsics::VisitBindingPat(BindingPattern*){};
void MarkIntrinsics::VisitDiscardingPat(DiscardingPattern*){};
void MarkIntrinsics::VisitLiteralPat(LiteralPattern*){};
void MarkIntrinsics::VisitVariantPat(VariantPattern*){};

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitYield(YieldExpression* node) {
  node->yield_value_ = Eval(node->yield_value_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitReturn(ReturnExpression* node) {
  node->return_value_ = Eval(node->return_value_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitAssign(AssignExpression* node) {
  node->target_ = Eval(node->target_)->as<Expression>();
  node->value_ = Eval(node->value_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitSeqExpr(SeqExpression* node) {
  node->expr_ = Eval(node->expr_)->as<Expression>();
  return_value = node;
}

//////////////////////////////////////////////////////////////////////

void MarkIntrinsics::VisitComparison(ComparisonExpression* node) {
  node->left_ = Eval(node->left_)->as<Expression>();
  node->right_ = Eval(node->right_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitBinary(BinaryExpression* node) {
  node->left_ = Eval(node->left_)->as<Expression>();
  node->right_ = Eval(node->right_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitUnary(UnaryExpression* node) {
  node->operand_ = Eval(node->operand_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitDeref(DereferenceExpression* node) {
  node->operand_ = Eval(node->operand_)->as<Expression>();

  return_value = node;
}

void MarkIntrinsics::VisitAddressof(AddressofExpression* node) {
  node->operand_ = Eval(node->operand_)->as<Expression>();

  return_value = node;
}

void MarkIntrinsics::VisitIf(IfExpression* node) {
  node->condition_ = Eval(node->condition_)->as<Expression>();
  node->true_branch_ = Eval(node->true_branch_)->as<Expression>();
  node->false_branch_ = Eval(node->false_branch_)->as<Expression>();

  return_value = node;
}

void MarkIntrinsics::VisitMatch(MatchExpression* node) {
  node->against_ = Eval(node->against_)->as<Expression>();

  for (auto& [pat, expr] : node->patterns_) {
    expr = Eval(expr)->as<Expression>();
  }

  return_value = node;
}

void MarkIntrinsics::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    node->allocation_size_ = Eval(node->allocation_size_)->as<Expression>();
  }

  if (node->initial_value_) {
    node->initial_value_ = Eval(node->initial_value_)->as<Expression>();
  }

  return_value = node;
}

void MarkIntrinsics::VisitLet(LetExpression* node) {
  node->value_ = Eval(node->value_)->as<Expression>();

  node->rest_ = node->rest_ ? Eval(node->rest_)->as<Expression>()  //
                            : nullptr;

  node->else_rest_ = node->else_rest_
                         ? Eval(node->else_rest_)->as<Expression>()  //
                         : nullptr;
}

void MarkIntrinsics::VisitBlock(BlockExpression* node) {
  node->expr_ = Eval(node->expr_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitIndex(IndexExpression* node) {
  node->indexed_expr_ = Eval(node->indexed_expr_)->as<Expression>();
  return_value = node;
}

void MarkIntrinsics::VisitFnCall(FnCallExpression* node) {
  for (auto& a : node->arguments_) {
    a = Eval(a)->as<Expression>();
  }

  if (intrinsics_table.contains(node->GetFunctionName())) {
    return_value = new IntrinsicCall(node);
  } else {
    return_value = node;
  }
}

void MarkIntrinsics::VisitIntrinsic(IntrinsicCall*) {
  std::abort();
}

void MarkIntrinsics::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  for (auto& mem : node->initializers_) {
    if (auto& init = mem.init) {
      init = Eval(init)->as<Expression>();
    }
  }

  return_value = node;
}

void MarkIntrinsics::VisitFieldAccess(FieldAccessExpression* node) {
  node->struct_expression_ = Eval(node->struct_expression_)->as<Expression>();

  return_value = node;
}

void MarkIntrinsics::VisitVarAccess(VarAccessExpression* node) {
  return_value = node;
}

void MarkIntrinsics::VisitLiteral(LiteralExpression* node) {
  return_value = node;
}

void MarkIntrinsics::VisitTypecast(TypecastExpression* node) {
  node->expr_ = Eval(node->expr_)->as<Expression>();
  return_value = node;
}

}  // namespace ast::elaboration
