#include <types/check/algorithm_w.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>
#include <lex/token.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitTypeDecl(TypeDeclStatement* node) {
  // No-op
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitVarDecl(VarDeclStatement* node) {
  return_value = Eval(node->value_);

  if (node->annotation_) {
    Unify(return_value, node->annotation_);
  } else {
    node->annotation_ = return_value;
  }
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitFunDecl(FunDeclStatement* node) {
  if (!node->body_)
    return;

  current_context_ = node->layer_;
  return_value = Eval(node->body_);
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitYield(YieldStatement* node) {
  Eval(node->yield_value_);
  return_value = &builtin_unit;
}

void AlgorithmW::VisitReturn(ReturnStatement* node) {
  Eval(node->return_value_);
  return_value = &builtin_unit;
}

void AlgorithmW::VisitAssignment(AssignmentStatement* node) {
  auto val_ty = Eval(node->value_);
  auto targ_ty = Eval(node->target_);
  Unify(val_ty, targ_ty);
}

void AlgorithmW::VisitExprStatement(ExprStatement* node) {
  Eval(node->expr_);
  return_value = &builtin_unit;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitComparison(ComparisonExpression* node) {
  Unify(Eval(node->left_), &builtin_int);
  Unify(Eval(node->right_), &builtin_int);
  return_value = &builtin_bool;
}

void AlgorithmW::VisitBinary(BinaryExpression* node) {
  Unify(Eval(node->left_), &builtin_int);
  Unify(Eval(node->right_), &builtin_int);
  return_value = &builtin_int;
}

void AlgorithmW::VisitUnary(UnaryExpression* node) {
  auto result = Eval(node->operand_);

  switch (node->operator_) {
    case lex::TokenType::MINUS:
    case lex::TokenType::NOT:

    default:
      std::abort();
  }

  Unify(Eval(node->operand_), &builtin_int);
  return_value = result;
}

void AlgorithmW::VisitDeref(DereferenceExpression* node) {
  node->operand_;
}

void AlgorithmW::VisitAddressof(AddressofExpression* node) {
  node->operand_;
}

void AlgorithmW::VisitIf(IfExpression* node) {
  node->condition_;
}

void AlgorithmW::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    node->allocation_size_;
  }
}

void AlgorithmW::VisitBlock(BlockExpression* node) {
}

void AlgorithmW::VisitFnCall(FnCallExpression* node) {
}

void AlgorithmW::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  for (auto val : node->values_) {
    val;
  }
}

void AlgorithmW::VisitFieldAccess(FieldAccessExpression* node) {
}

void AlgorithmW::VisitVarAccess(VarAccessExpression* node) {
}

void AlgorithmW::VisitLiteral(LiteralExpression* node) {
}

void AlgorithmW::VisitTypecast(TypecastExpression* node) {
  node->expr_;
}

}  // namespace types::check
