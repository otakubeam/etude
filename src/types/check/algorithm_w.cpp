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
  auto symbol = node->layer_->bindings.symbol_map.at(node->GetVarName());

  auto ty = symbol->GetType();
  Unify(ty, Eval(node->value_));
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitFunDecl(FunDeclStatement* node) {
  if (!node->body_) {
    return;
  }

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
  auto symbol = find->bindings.symbol_map.at(fn_name);
  auto ty = MakeFunType(std::move(param_pack), MakeTypeVar());

  Unify(ty, symbol->GetType());

  Unify(Eval(node->body_), ty->as_fun.result_type);

  // Top-level: Generalize

  if (node->layer_->level == 1) {
    Generalize(ty);
  }

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
  auto e = Eval(node->left_);
  auto e2 = Eval(node->right_);

  switch (node->operator_.type) {
    case lex::TokenType::EQUALS:
      deferred_checks_.push({.tag = TraitTags::EQ, .bound = e, .none = {}});
      break;

    case lex::TokenType::LT:
    case lex::TokenType::GT:
      deferred_checks_.push({.tag = TraitTags::ORD, .bound = e, .none = {}});
      break;

    case lex::TokenType::LE:
    case lex::TokenType::GE:
      deferred_checks_.push({.tag = TraitTags::EQ, .bound = e, .none = {}});
      deferred_checks_.push({.tag = TraitTags::ORD, .bound = e, .none = {}});
      break;

    default:
      std::abort();
  }

  Unify(e, e2);  // Do not implicitly convert types
  return_value = &builtin_bool;
}

void AlgorithmW::VisitBinary(BinaryExpression* node) {
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
  auto a = Eval(node->operand_);
  auto b = MakeTypeVar();
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

  return_value = node->type_;
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
    auto symbol = find->bindings.symbol_map.at(node->fn_name_);

    node->layer_->Print();

    auto ty = symbol->GetType();

    // Get new fresh variables for all type parameters

    KnownParams map = {};
    ty = Instantinate(ty, map);
    fmt::print("Instantiated type: {}\n", FormatType(*ty));

    deferred_checks_.push({
        .tag = TraitTags::CALLABLE,
        .bound = ty,
        .none = {},
    });

    auto& pack = ty->as_fun.param_pack;
    auto& args = node->arguments_;

    if (pack.size() != args.size()) {
      throw "Function call size mismatch";
    };

    for (size_t i = 0; i < args.size(); i++) {
      Unify(Eval(args[i]), pack[i]);
    }

    return_value = symbol->GetType()->as_fun.result_type;

  } else {
    FMT_ASSERT(false, "Why are you here?");
  }
}

void AlgorithmW::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  auto find = node->layer_->Find(node->struct_name_);
  auto symbol = find->bindings.symbol_map.at(node->struct_name_);
  auto ty = symbol->GetType();

  auto& members = ty->as_struct.first;
  auto& values = node->values_;

  if (members.size() != values.size()) {
    throw "Struct construction size mismatch";
  }

  for (size_t i = 0; i < values.size(); i++) {
    Unify(Eval(values[i]), members[i].ty);
  }
}

void AlgorithmW::VisitFieldAccess(FieldAccessExpression* node) {
  auto e = Eval(node->struct_expression_);

  return_value = node->type_ = MakeTypeVar();

  deferred_checks_.push(Trait{.tag = TraitTags::HAS_FIELD,
                              .bound = FindLeader(e),
                              .has_field = {.field_name = node->field_name_,
                                            .field_type = node->type_}});
}

void AlgorithmW::VisitVarAccess(VarAccessExpression* node) {
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

  FMT_ASSERT(node->type_, "Explicit cast must provide type");

  deferred_checks_.push(Trait{.tag = TraitTags::CONVERTIBLE_TO,
                              .bound = FindLeader(e),
                              .convertible_to = {.to_type = node->type_}});
  return_value = node->type_;
}

}  // namespace types::check
