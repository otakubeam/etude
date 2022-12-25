#include <types/instantiate/instantiator.hpp>
#include <types/constraints/solver.hpp>

#include <ast/patterns.hpp>

#include <lex/token.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitTypeDecl(TypeDeclStatement* node) {
  auto n = new TypeDeclStatement{*node};
  n->body_ = Instantinate(n->body_, poly_to_mono_);

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitVarDecl(VarDeclStatement* node) {
  auto n = new VarDeclStatement{*node};

  n->annotation_ = Instantinate(n->annotation_, poly_to_mono_);
  n->value_ = Eval(n->value_)->as<Expression>();

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitFunDecl(FunDeclStatement* node) {
  auto n = new FunDeclStatement{*node};
  n->type_ = Instantinate(n->type_, poly_to_mono_);
  if (n->body_) {
    n->body_ = Eval(n->body_)->as<Expression>();
  }

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitTraitDecl(TraitDeclaration* node) {
  std::abort();
  (void)node;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitBindingPat(BindingPattern* node) {
  auto n = new BindingPattern{*node};

  n->type_ = Instantinate(FindLeader(node->type_), poly_to_mono_);
  // node->type_ |> FindLeader |> Instantinate(poly_to_mono_);

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitDiscardingPat(DiscardingPattern* node) {
  auto n = new DiscardingPattern{*node};

  n->type_ = Instantinate(FindLeader(node->type_), poly_to_mono_);
  // node->type_ |> FindLeader() |> Instantinate(poly_to_mono_);

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitLiteralPat(LiteralPattern* node) {
  return_value = new LiteralPattern{*node};
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitVariantPat(VariantPattern* node) {
  auto n = new VariantPattern{*node};

  if (auto& inner = n->inner_pat_) {
    inner = Eval(inner)->as<Pattern>();
  }

  n->type_ = Instantinate(FindLeader(node->type_), poly_to_mono_);

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitYield(YieldStatement* node) {
  auto n = new YieldStatement{*node};
  n->yield_value_ = Eval(n->yield_value_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitReturn(ReturnStatement* node) {
  auto n = new ReturnStatement{*node};
  n->return_value_ = Eval(n->return_value_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitAssignment(AssignmentStatement* node) {
  auto n = new AssignmentStatement{*node};

  n->target_ = Eval(n->target_)->as<LvalueExpression>();
  n->value_ = Eval(n->value_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitExprStatement(ExprStatement* node) {
  auto n = new ExprStatement{*node};
  n->expr_ = Eval(n->expr_)->as<Expression>();

  return_value = n;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::VisitComparison(ComparisonExpression* node) {
  auto n = new ComparisonExpression{*node};

  n->left_ = Eval(n->left_)->as<Expression>();
  n->right_ = Eval(n->right_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitBinary(BinaryExpression* node) {
  auto n = new BinaryExpression{*node};

  n->left_ = Eval(n->left_)->as<Expression>();
  n->right_ = Eval(n->right_)->as<Expression>();

  n->type_ = n->left_->GetType();

  return_value = n;
}

void TemplateInstantiator::VisitUnary(UnaryExpression* node) {
  auto n = new UnaryExpression{*node};
  n->operand_ = Eval(n->operand_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitDeref(DereferenceExpression* node) {
  auto n = new DereferenceExpression{*node};

  n->type_ = Instantinate(n->type_, poly_to_mono_);

  n->operand_ = Eval(n->operand_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitAddressof(AddressofExpression* node) {
  auto n = new AddressofExpression{*node};

  n->type_ = Instantinate(n->type_, poly_to_mono_);

  n->operand_ = Eval(n->operand_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitIf(IfExpression* node) {
  auto n = new IfExpression{*node};

  n->condition_ = Eval(n->condition_)->as<Expression>();
  n->true_branch_ = Eval(n->true_branch_)->as<Expression>();
  n->false_branch_ = Eval(n->false_branch_)->as<Expression>();

  return_value = n;
}

void TemplateInstantiator::VisitMatch(MatchExpression* node) {
  auto n = new MatchExpression{*node};

  n->against_ = Eval(n->against_)->as<Expression>();

  for (auto& [pat, expr] : n->patterns_) {
    pat = Eval(pat)->as<Pattern>();
    expr = Eval(expr)->as<Expression>();
  }

  return_value = n;
}

void TemplateInstantiator::VisitNew(NewExpression* node) {
  auto n = new NewExpression{*node};

  n->type_ = Instantinate(node->type_, poly_to_mono_);

  n->underlying_ = Instantinate(node->underlying_, poly_to_mono_);

  if (auto& alloc = n->allocation_size_) {
    alloc = Eval(alloc)->as<Expression>();
  }

  if (auto& init = node->initial_value_) {
    init = Eval(init)->as<Expression>();
  }

  return_value = n;
}

void TemplateInstantiator::VisitBlock(BlockExpression* node) {
  auto n = new BlockExpression{*node};

  for (auto& s : n->stmts_) {
    s = Eval(s)->as<Statement>();
  }

  if (n->final_) {
    n->final_ = Eval(n->final_)->as<Expression>();
  }

  return_value = n;
}

void TemplateInstantiator::VisitFnCall(FnCallExpression* node) {
  auto n = new FnCallExpression{*node};

  for (auto& a : n->arguments_) {
    a = Eval(a)->as<Expression>();
    MaybeSaveForIL(a->GetType());
  }

  // Get result type

  // TODO: do I need this?
  // n->callable_ = Eval(node->callable_)->as<Expression>();

  n->callable_type_ = Instantinate(node->callable_type_, poly_to_mono_);

  //  n->layer_ = call_context_;

  MaybeSaveForIL(n->GetType());

  fmt::print(stderr, "{}\n", FormatType(*n->callable_type_));
  fmt::print(stderr, "Adding a node to the queue\n");

  instantiation_quque_.push_back(n);

  return_value = n;
}

void TemplateInstantiator::VisitIntrinsic(IntrinsicCall* node) {
  auto n = new IntrinsicCall{*node};
  for (auto& a : n->arguments_) {
    a = Eval(a)->as<Expression>();
  }
  return_value = n;
}

void TemplateInstantiator::VisitCompoundInitalizer(
    CompoundInitializerExpr* node) {
  auto n = new CompoundInitializerExpr{*node};

  for (auto& mem : n->initializers_) {
    if (mem.init) {
      mem.init = Eval(mem.init)->as<Expression>();
    }
  }

  n->type_ = Instantinate(node->type_, poly_to_mono_);

  return_value = n;
}

void TemplateInstantiator::VisitFieldAccess(FieldAccessExpression* node) {
  auto n = new FieldAccessExpression{*node};
  n->struct_expression_ = Eval(node->struct_expression_)->as<Expression>();

  n->type_ = Instantinate(node->type_, poly_to_mono_);

  return_value = n;
}

void TemplateInstantiator::VisitVarAccess(VarAccessExpression* node) {
  auto n = new VarAccessExpression{*node};

  n->type_ = Instantinate(node->GetType(), poly_to_mono_);

  if (n->type_->tag == TypeTag::TY_FUN) {
    function_ptrs_.push_back(n);
  }

  return_value = n;
}

void TemplateInstantiator::VisitLiteral(LiteralExpression* node) {
  return_value = new LiteralExpression{*node};
}

void TemplateInstantiator::VisitTypecast(TypecastExpression* node) {
  auto n = new TypecastExpression{*node};

  n->type_ = Instantinate(node->type_, poly_to_mono_);

  return_value = n;
}

}  // namespace types::check
