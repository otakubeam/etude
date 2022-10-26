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
    param_pack.push_back(new Type{});

    auto find = node->layer_->Find(f);
    auto symbol = find->bindings.symbol_map.at(f);

    // Allocate new type variable
    if (!symbol->as_varbind.type) {
      symbol->as_varbind.type = new types::Type{};
    }

    Unify(symbol->as_varbind.type, param_pack.back());
  }

  fmt::print("GOOD\n");

  auto ty = node->layer_->parent->functions.symbol_map.at(node->GetFunctionName())
                ->as_fn_sym.type =
      new Type{.tag = TypeTag::TY_FUN,
               .as_fun = {.param_pack = std::move(param_pack),
                          .result_type = new Type{}}};


  fmt::print("GOOD\n");

  return_value = Eval(node->body_);

  Unify(return_value, ty->as_fun.result_type);

  // Generalize

  fmt::print("Fn type {}\n", (void*)ty);
  fmt::print("Fn type {}\n", types::FormatType(*ty));

  fmt::print("Almost done\n");
  fmt::print("{}\n", FormatType(*return_value));
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
  // fun use_ptr p = {    <<<--- 1) p :: a
  //    *p                       2) a ~ *b
  // };                          3) *p :: b
  //
  auto a = Eval(node->operand_);
  auto b = new Type{};
  Unify(a, new Type{.tag = TypeTag::TY_PTR, .as_ptr = {.underlying = b}});
  return_value = b;
}

void AlgorithmW::VisitAddressof(AddressofExpression* node) {
  // Add constraint on operand type?
  return_value = new Type{
      .tag = TypeTag::TY_PTR,
      .as_ptr = {.underlying = Eval(node->operand_)},
  };
}

void AlgorithmW::VisitIf(IfExpression* node) {
  Unify(Eval(node->condition_), &builtin_bool);
  auto a = Eval(node->true_branch_);
  Unify(a, Eval(node->false_branch_));
  return_value = a;
}

void AlgorithmW::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    Unify(Eval(node->allocation_size_), &builtin_int);
  }

  return_value = new Type{
      .tag = TypeTag::TY_PTR,
      .as_ptr = {.underlying = node->type_},
  };
}

void AlgorithmW::VisitBlock(BlockExpression* node) {
  for (auto stmt : node->stmts_) {
    Eval(stmt);
  }

  if (node->final_) {
    fmt::print("Calc final\n");
    return_value = Eval(node->final_);
    fmt::print("Calced\n");
  } else {
    return_value = &builtin_unit;
  }
}

void AlgorithmW::VisitFnCall(FnCallExpression* node) {
  if (!node->fn_name_.empty()) {
    // Handle this case separately

    if (!node->layer_->functions.symbol_map.at(node->fn_name_)
             ->as_fn_sym.type) {
      auto n = node->layer_->functions.symbol_map.at(node->fn_name_)
                   ->as_fn_sym.type = new Type{};

      if (n->as_fun.param_pack.size() != node->arguments_.size()) {
        throw "Error";
      };
      // Eval(node->callable_);  // TODO: Unify?
      for (size_t i = 0; i < node->arguments_.size(); i++) {
        Unify(Eval(node->arguments_[i]), n->as_fun.param_pack[i]);
      }

      // Can I just return  n->as_fun.result_type  ?
      return_value = n->as_fun.result_type;
    }
  } else {
    std::abort();
  }
}

void AlgorithmW::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  // ? node->layer_->symbol_map.contains(node->struct_name_);

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
    node->type_ = new Type{};
  }

  return_value = node->type_;

  // Constraints are registered for post-resolution

  // WHERE DO I PUT THIS?
  (void)Trait{.tag = TraitTags::HAS_FIELD,
              .has_field = {.field_name = node->field_name_,
                            .field_type = node->type_}};
}

void AlgorithmW::VisitVarAccess(VarAccessExpression* node) {
  if (auto cxt = node->layer_->Find(node->name_)) {
    // If we were not provided user annotation
    node->type_ = cxt->bindings.symbol_map.at(node->name_)->as_varbind.type;

    return_value = node->type_;
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
      return_value = new Type{.tag = TypeTag::TY_PTR,
                              .as_ptr = {.underlying = &builtin_char}};
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
  FMT_ASSERT(node->type_, "Explicit cast must provide type");

  auto e = Eval(node->expr_);
  (void)e;

  // TODO: push constraint that e must be convertible to node->type_

  return_value = node->type_;
}

}  // namespace types::check
