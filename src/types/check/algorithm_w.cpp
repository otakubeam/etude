#include <types/check/algorithm_w.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>
#include <lex/token.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitTypeDecl(TypeDeclStatement*) {
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

  // Build param pack

  std::vector<Type*> param_pack;

  for (auto f : node->formals_) {
    param_pack.push_back(MakeTypeVar());

    auto find = node->layer_->Find(f);
    auto symbol = find->bindings.symbol_map.at(f);

    Unify(symbol->GetType(), param_pack.back());
  }

  // Make function type

  auto fn_name = node->GetFunctionName();
  auto find = node->layer_->Find(fn_name);
  auto symbol = find->functions.symbol_map.at(fn_name);
  auto ty = MakeFunType(std::move(param_pack), MakeTypeVar());

  Unify(ty, symbol->GetType());

  Unify(Eval(node->body_), ty->as_fun.result_type);

  // TODO: Generics, Generalize

  // Return the type of this declaration ??

  return_value = ty;
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
  auto value_ty = Eval(node->value_);
  auto target_ty = Eval(node->target_);
  Unify(value_ty, target_ty);
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
  // Unify(, &builtin_int);
  Unify(Eval(node->right_), &builtin_int);
  return_value = Eval(node->left_);
}

void AlgorithmW::VisitUnary(UnaryExpression* node) {
  auto result = Eval(node->operand_);

  switch (node->operator_.type) {
    case lex::TokenType::MINUS:
      Unify(Eval(node->operand_), &builtin_int);
      break;

    case lex::TokenType::NOT:
      Unify(Eval(node->operand_), &builtin_bool);
      break;

    default:
      std::abort();
  }

  return_value = result;
}

void AlgorithmW::VisitDeref(DereferenceExpression* node) {
  // An example:
  //
  //     fun use_ptr p = {    <<<--- 1) p :: a
  //        *p                       2) a ~ *b
  //     };                          3) *p :: b
  //
  fmt::print("Inside deref\n");
  auto a = Eval(node->operand_);
  fmt::print("Inside deref\n");
  // fmt::print("{}", (void*)a);
  auto b = MakeTypeVar();
  fmt::print("Inside deref\n");
  Unify(a, MakeTypePtr(b));

  return_value = b;
}

void AlgorithmW::VisitAddressof(AddressofExpression* node) {
  return_value = MakeTypePtr(Eval(node->operand_));
}

void AlgorithmW::VisitIf(IfExpression* node) {
  Unify(Eval(node->condition_), &builtin_bool);
  auto true_ty = Eval(node->true_branch_);
  Unify(true_ty, Eval(node->false_branch_));
  return_value = true_ty;
}

void AlgorithmW::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    Unify(Eval(node->allocation_size_), &builtin_int);
  }

  return_value = node->type_;  // Set in ContextBuilder
}

void AlgorithmW::VisitBlock(BlockExpression* node) {
  for (auto stmt : node->stmts_) {
    Eval(stmt);
  }

  if (node->final_) {
    return_value = Eval(node->final_);
  } else {
    return_value = &builtin_unit;
  }
}

void AlgorithmW::VisitFnCall(FnCallExpression* node) {
  if (!node->fn_name_.empty()) {
    // Handle this case separately

    auto find = node->layer_->Find(node->fn_name_);
    auto symbol = find->functions.symbol_map.at(node->fn_name_);

    node->layer_->Print();

    auto ty = symbol->GetType();

    auto& pack = ty->as_fun.param_pack;
    auto& args = node->arguments_;

    if (pack.size() != args.size()) {
      fmt::print("{} {}\n", pack.size(), args.size());
      throw "Error";
    };

    // Eval(node->callable_);  // TODO: Constrain?

    for (size_t i = 0; i < args.size(); i++) {
      // TODO(Generics): specialize here, not unify
      Unify(Eval(args[i]), pack[i]);
    }

    return_value = symbol->GetType()->as_fun.result_type;

    fmt::print("Ok here\n");
  } else {
    FMT_ASSERT(false, "Why are you here?");
  }
}

void AlgorithmW::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  // ? node->layer_->symbol_map.contains(node->struct_name_);

  // Basically check that all fields match

  auto& v = node->layer_->symbol_map.at("wef")->as_struct.type->as_struct.first;

  if (v.size() != node->values_.size()) {
    throw;
  }

  for (size_t i = 0; i < v.size(); i++) {
    Unify(Eval(node->values_[i]), v[i].ty);
  }
}

void AlgorithmW::VisitFieldAccess(FieldAccessExpression* node) {
  auto e = Eval(node->struct_expression_);
  (void)e;

  if (!node->type_) {
    node->type_ = MakeTypeVar();
  }

  return_value = node->type_;

  // Constraints are registered for post-resolution

  // WHERE DO I PUT THIS?
  (void)Trait{.tag = TraitTags::HAS_FIELD,
              .has_field = {.field_name = node->field_name_,
                            .field_type = node->type_}};
}

void AlgorithmW::VisitVarAccess(VarAccessExpression* node) {
  FMT_ASSERT(node->layer_, "Fail W");
  fmt::print("{}\n", node->name_.GetName());
  node->layer_->Print();
  if (auto cxt = node->layer_->Find(node->name_)) {
    auto symbol = cxt->bindings.symbol_map.at(node->name_);
    return_value = symbol->GetType();
    return;
  }

  // TODO: also handle functions

  std::abort();
}

void AlgorithmW::VisitLiteral(LiteralExpression* node) {
  switch (node->token_.type) {
    case lex::TokenType::NUMBER:
      return_value = &builtin_int;
      break;

    case lex::TokenType::STRING:
      return_value = MakeTypePtr(&builtin_char);
      break;

    case lex::TokenType::UNIT:
      return_value = &builtin_unit;
      break;

    case lex::TokenType::TRUE:
    case lex::TokenType::FALSE:
      return_value = &builtin_bool;
      break;

    default:
      FMT_ASSERT(false, "Typechecking unknown literal");
  }

  node->type_ = return_value;
}

void AlgorithmW::VisitTypecast(TypecastExpression* node) {
  auto e = Eval(node->expr_);
  (void)e;

  // TODO: push constraint that e must be convertible to node->type_

  FMT_ASSERT(node->type_, "Explicit cast must provide type");
  return_value = node->type_;
}

}  // namespace types::check
