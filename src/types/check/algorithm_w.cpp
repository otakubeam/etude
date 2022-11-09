#include <types/constraints/solver.hpp>
#include <types/check/algorithm_w.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>
#include <lex/token.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

void AlgorithmW::PushEqual(Type* a, Type* b) {
  deferred_checks_.push_back(Trait{
      .tag = TraitTags::TYPES_EQ,
      .types_equal = {.a = a, .b = b},
  });
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitTypeDecl(TypeDeclStatement*) {
  // No-op
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitVarDecl(VarDeclStatement* node) {
  auto symbol = node->layer_->RetrieveSymbol(node->GetVarName());

  auto ty = symbol->GetType();

  PushEqual(ty, Eval(node->value_));
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitFunDecl(FunDeclStatement* node) {
  if (!node->body_) {
    return;
  }

  // Build param pack

  std::vector<Type*> param_pack;

  for (auto f : node->formals_) {
    param_pack.push_back(MakeTypeVar(node->layer_));

    auto symbol = node->layer_->RetrieveSymbol(f);

    PushEqual(symbol->GetType(), param_pack.back());
  }

  // Make function type

  auto fn_name = node->GetFunctionName();
  auto symbol = node->layer_->RetrieveSymbol(fn_name);
  auto ty = MakeFunType(std::move(param_pack), MakeTypeVar());
  SetTyContext(ty, node->layer_);

  PushEqual(ty, symbol->GetType());

  PushEqual(Eval(node->body_), ty->as_fun.result_type);

  // Resolve constraints

  constraints::ConstraintSolver solver{std::move(deferred_checks_)};
  deferred_checks_ = {};  // Empty checks
  solver.Solve();

  // Top-level: Generalize

  if (node->layer_->level == 1) {
    // Is this correct? What if some other type points at me as a leader?
    Generalize(ty);
  }

  node->type_ = return_value = ty;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitYield(YieldStatement* node) {
  Eval(node->yield_value_);
  return_value = &builtin_unit;
}

void AlgorithmW::VisitReturn(ReturnStatement* node) {
  auto find = node->layer_->RetrieveSymbol(node->this_fun);

  std::vector<Type*> args;
  for (size_t i = 0; i < find->as_fn_sym.argnum; i++) {
    args.push_back(MakeTypeVar(node->layer_));
  }

  auto ty = MakeFunType(std::move(args), Eval(node->return_value_));
  SetTyContext(ty, node->layer_);

  PushEqual(find->GetType(), ty);

  return_value = &builtin_unit;
}

void AlgorithmW::VisitAssignment(AssignmentStatement* node) {
  auto value_ty = Eval(node->value_);
  auto target_ty = Eval(node->target_);
  PushEqual(value_ty, target_ty);
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
      deferred_checks_.push_back(
          {.tag = TraitTags::EQ, .bound = e, .none = {}});
      break;

    case lex::TokenType::LT:
    case lex::TokenType::GT:
      deferred_checks_.push_back(
          {.tag = TraitTags::ORD, .bound = e, .none = {}});
      break;

    case lex::TokenType::LE:
    case lex::TokenType::GE:
      deferred_checks_.push_back(
          {.tag = TraitTags::EQ, .bound = e, .none = {}});
      deferred_checks_.push_back(
          {.tag = TraitTags::ORD, .bound = e, .none = {}});
      break;

    default:
      std::abort();
  }

  PushEqual(e, e2);  // Do not implicitly convert types
  return_value = &builtin_bool;
}

void AlgorithmW::VisitBinary(BinaryExpression* node) {
  PushEqual(Eval(node->right_), &builtin_int);

  node->type_ = return_value = Eval(node->left_);

  deferred_checks_.push_back(Trait{
      .tag = TraitTags::ADD,
      .bound = return_value,
      .none = {},
  });
}

void AlgorithmW::VisitUnary(UnaryExpression* node) {
  auto result = Eval(node->operand_);

  switch (node->operator_.type) {
    case lex::TokenType::MINUS:
      PushEqual(Eval(node->operand_), &builtin_int);
      break;

    case lex::TokenType::NOT:
      PushEqual(Eval(node->operand_), &builtin_bool);
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
  PushEqual(a, MakeTypePtr(b));
  node->type_ = return_value = FindLeader(b);
}

void AlgorithmW::VisitAddressof(AddressofExpression* node) {
  node->type_ = return_value = MakeTypePtr(Eval(node->operand_));
  SetTyContext(node->type_, node->layer_);
}

void AlgorithmW::VisitIf(IfExpression* node) {
  PushEqual(Eval(node->condition_), &builtin_bool);
  auto true_ty = Eval(node->true_branch_);
  PushEqual(true_ty, Eval(node->false_branch_));
  return_value = true_ty;
}

void AlgorithmW::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    PushEqual(Eval(node->allocation_size_), &builtin_int);
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
  if (node->fn_name_.empty()) {
    FMT_ASSERT(false, "Why are you here?");
  }

  auto symbol = node->layer_->RetrieveSymbol(node->fn_name_);

  if (!symbol) {
    node->layer_->Print();
    throw std::runtime_error{
        fmt::format("Could not find function {} at loc {}",  //
                    node->fn_name_, node->GetLocation().Format())};
  }

  auto ty = symbol->GetType();
  auto ctx = ty->typing_context_;

  // Get new fresh variables for all type parameters

  KnownParams map = {};
  ty = Instantinate(ty, map);
  SetTyContext(ty, ctx);

  // Build function type bases on arguments

  std::vector<Type*> result;
  for (auto& a : node->arguments_) {
    result.push_back(Eval(a));
  }
  auto result_ty = MakeTypeVar(ctx);
  auto f = MakeFunType(std::move(result), result_ty);

  SetTyContext(f, ty->typing_context_);
  node->callable_type_ = f;

  // Constrain

  deferred_checks_.push_back({
      .tag = TraitTags::CALLABLE,
      .bound = ty,
      .none = {},
  });

  PushEqual(f, ty);

  return_value = result_ty;
}

void AlgorithmW::VisitIntrinsic(IntrinsicCall* node) {
  if (node->arguments_.size() > 1) {
    throw "Too many arguments";
  }

  switch (node->intrinsic) {
    case ast::elaboration::Intrinsic::PRINT:
      // TODO: Show typeclass?
      PushEqual(Eval(node->arguments_.at(0)), MakeTypePtr(&builtin_char));
      return_value = &builtin_unit;
      break;

    case ast::elaboration::Intrinsic::ASSERT:
      PushEqual(Eval(node->arguments_.at(0)), &builtin_bool);
      return_value = &builtin_unit;
      break;

    case ast::elaboration::Intrinsic::IS_NULL:
      PushEqual(Eval(node->arguments_.at(0)),
                MakeTypePtr(MakeTypeVar(node->layer_)));
      return_value = &builtin_bool;
      break;

    default:
      std::abort();
  }
}

void AlgorithmW::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  node->type_ = MakeTypeVar(node->layer_);

  for (auto& mem : node->initializers_) {
    deferred_checks_.push_back(Trait{.tag = TraitTags::HAS_FIELD,
                                     .bound = node->type_,
                                     .has_field = {
                                         .field_name = mem.field,
                                         .field_type = Eval(mem.init),
                                     }});
  }

  return_value = node->type_;
}

void AlgorithmW::VisitFieldAccess(FieldAccessExpression* node) {
  auto e = Eval(node->struct_expression_);

  e = FindLeader(e);

  return_value = node->type_ = MakeTypeVar();

  deferred_checks_.push_back(
      Trait{.tag = TraitTags::HAS_FIELD,
            .bound = e,
            .has_field = {.field_name = node->field_name_,
                          .field_type = node->type_}});
}

void AlgorithmW::VisitVarAccess(VarAccessExpression* node) {
  if (auto symbol = node->layer_->RetrieveSymbol(node->name_)) {
    auto ty = symbol->GetType();

    node->type_ = ty;

    KnownParams p = {};
    return_value = ty->tag == TypeTag::TY_FUN ? Instantinate(ty, p) : ty;

    return;
  } else {
    throw std::runtime_error{
        fmt::format("Could not find {}", node->name_.GetName())};
  }

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

  deferred_checks_.push_back(Trait{.tag = TraitTags::CONVERTIBLE_TO,
                                   .bound = FindLeader(e),
                                   .convertible_to = {.to_type = node->type_}});
  return_value = node->type_;
}

}  // namespace types::check
