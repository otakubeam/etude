#include <types/constraints/solver.hpp>
#include <types/constraints/generate/algorithm_w.hpp>

#include <ast/declarations.hpp>
#include <ast/patterns.hpp>
#include <lex/token.hpp>

namespace types::constraints::generate {

//////////////////////////////////////////////////////////////////////

void AlgorithmW::PushEqual(lex::Location loc, Type* a, Type* b) {
  if (solver_.Unify(a, b)) {
    return;  // If can unify right away, then do it
  }
  work_queue_.push_back(MakeTyEqTrait(a, b, loc));
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitTypeDecl(TypeDeclaration*) {
  // No-op
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitVarDecl(VarDeclaration* node) {
  auto symbol = node->layer_->RetrieveSymbol(node->GetName());

  auto ty = symbol->GetType();

  PushEqual(node->GetLocation(), ty, Eval(node->value_));
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitFunDecl(FunDeclaration* node) {
  if (!node->body_) {
    return;
  }

  current_function_ = node->GetName();

  // Build param pack

  std::vector<Type*> param_pack;

  for (auto f : node->formals_) {
    param_pack.push_back(MakeTypeVar(node->layer_));

    auto symbol = node->layer_->RetrieveSymbol(f);

    PushEqual(node->GetLocation(), symbol->GetType(), param_pack.back());
  }

  // Make function type

  auto fn_name = node->GetName();
  auto symbol = node->layer_->RetrieveSymbol(fn_name);
  auto ty = MakeFunType(std::move(param_pack), MakeTypeVar());
  SetTyContext(ty, node->layer_);

  if (symbol->sym_type == ast::scope::SymbolType::TRAIT_METHOD) {
    KnownParams map = {};
    auto method_ty = Instantinate(symbol->GetType(), map);

    PushEqual(node->GetLocation(), ty, method_ty);
    if (node->type_) {
      PushEqual(node->GetLocation(), ty, node->type_);
    }
  } else {
    PushEqual(node->GetLocation(), ty, symbol->GetType());
  }

  PushEqual(node->GetLocation(), Eval(node->body_), ty->as_fun.result_type);

  node->type_ = return_value = ty;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitTraitDecl(TraitDeclaration* node) {
  for (auto decl : node->assoc_items_) {
    decl->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitImplDecl(ImplDeclaration* node) {
  for (auto decl : node->assoc_items_) {
    decl->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitModuleDecl(ModuleDeclaration* node) {
  auto mod = node->layer_->RetrieveSymbol(node->GetName());
  FMT_ASSERT(mod->sym_type == ast::scope::SymbolType::MODULE, "");

  for (auto i = mod->as_module->functions; i; i = i->next) {
    Eval(i->definition);
  }

  for (auto i = mod->as_module->traits; i; i = i->next) {
    Eval(i->me);
  }

  for (auto i = mod->as_module->impls; i; i = i->next_) {
    Eval(i);
  }
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitBindingPat(BindingPattern* node) {
  auto symbol = node->layer_->RetrieveSymbol(node->name_);
  return_value = symbol->GetType();
}

void AlgorithmW::VisitDiscardingPat(DiscardingPattern* node) {
  return_value = MakeTypeVar(node->layer_);  // No type information
}

void AlgorithmW::VisitLiteralPat(LiteralPattern* node) {
  return_value = Eval(node->pat_);
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitVariantPat(VariantPattern* node) {
  auto inner = node->inner_pat_  //
                   ? Eval(node->inner_pat_)
                   : MakeTypeVar(node->layer_);

  node->type_ = MakeTypeVar(node->layer_);

  auto loc = node->GetLocation();
  auto trait = MakeHasFieldTrait(node->type_, node->name_, inner, loc);
  work_queue_.push_back(std::move(trait));

  return_value = node->type_;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitYield(YieldExpression* node) {
  Eval(node->yield_value_);
  return_value = &builtin_never;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitReturn(ReturnExpression* node) {
  auto find = node->layer_->RetrieveSymbol(current_function_);

  std::vector<Type*> args;

  auto argnum = find->as_fun->definition->formals_.size();

  for (size_t i = 0; i < argnum; i++) {
    args.push_back(MakeTypeVar(node->layer_));
  }

  auto ty = MakeFunType(std::move(args), Eval(node->return_value_));
  SetTyContext(ty, node->layer_);

  PushEqual(node->GetLocation(), find->GetType(), ty);

  return_value = &builtin_never;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitAssign(AssignExpression* node) {
  auto value_ty = Eval(node->value_);
  auto target_ty = Eval(node->target_);
  PushEqual(node->GetLocation(), value_ty, target_ty);

  return_value = &builtin_unit;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitSeqExpr(SeqExpression* node) {
  Eval(node->expr_);

  return_value = node->rest_ ? Eval(node->rest_)  //
                             : &builtin_unit;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitComparison(ComparisonExpression* node) {
  auto e = Eval(node->left_);
  auto e2 = Eval(node->right_);

  switch (node->operator_.type) {
    case lex::TokenType::NOT_EQ:
    case lex::TokenType::EQUALS:
      work_queue_.push_back(MakeEqTrait(e, node->GetLocation()));
      break;

    case lex::TokenType::LT:
    case lex::TokenType::GT:
      work_queue_.push_back(MakeOrdTrait(e, node->GetLocation()));
      break;

    case lex::TokenType::LE:
    case lex::TokenType::GE:
      work_queue_.push_back(MakeEqTrait(e, node->GetLocation()));
      work_queue_.push_back(MakeOrdTrait(e, node->GetLocation()));
      break;

    default:
      std::abort();
  }

  PushEqual(node->GetLocation(), e, e2);  // Do not implicitly convert types

  return_value = &builtin_bool;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitBinary(BinaryExpression* node) {
  PushEqual(node->GetLocation(), Eval(node->right_), &builtin_int);

  node->type_ = return_value = Eval(node->left_);

  work_queue_.push_back(Trait{
      .tag = TraitTags::ADD,
      .bound = return_value,
      .none = {},
      .location = node->GetLocation(),
  });
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitUnary(UnaryExpression* node) {
  auto result = Eval(node->operand_);

  switch (node->operator_.type) {
    case lex::TokenType::MINUS:
      PushEqual(node->GetLocation(), Eval(node->operand_), &builtin_int);
      break;

    case lex::TokenType::NOT:
      PushEqual(node->GetLocation(), Eval(node->operand_), &builtin_bool);
      break;

    default:
      std::abort();
  }

  return_value = result;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitDeref(DereferenceExpression* node) {
  // An example:
  //
  //     fun use_ptr p = {    <<<--- 1) p :: a
  //        *p                       2) a ~ *b
  //     };                          3) *p :: b
  //
  auto a = Eval(node->operand_);
  auto b = MakeTypeVar();
  PushEqual(node->GetLocation(), a, MakeTypePtr(b));
  node->type_ = return_value = FindLeader(b);
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitAddressof(AddressofExpression* node) {
  node->type_ = return_value = MakeTypePtr(Eval(node->operand_));
  SetTyContext(node->type_, node->layer_);
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitIf(IfExpression* node) {
  auto result_ty = MakeTypeVar();

  PushEqual(node->GetLocation(), Eval(node->condition_), &builtin_bool);
  PushEqual(node->GetLocation(), result_ty, Eval(node->true_branch_));
  PushEqual(node->GetLocation(), result_ty, Eval(node->false_branch_));

  node->type_ = return_value = result_ty;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitMatch(MatchExpression* node) {
  auto result_ty = MakeTypeVar();
  auto target_ty = Eval(node->against_);

  for (auto& [pat, expr] : node->patterns_) {
    PushEqual(node->GetLocation(), target_ty, Eval(pat));
    PushEqual(node->GetLocation(), result_ty, Eval(expr));
  }

  node->type_ = return_value = result_ty;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitNew(NewExpression* node) {
  if (node->allocation_size_) {
    PushEqual(node->GetLocation(), Eval(node->allocation_size_), &builtin_int);
  }

  if (node->initial_value_) {
    PushEqual(node->GetLocation(), Eval(node->initial_value_),
              node->underlying_);
  }

  return_value = node->type_;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitLet(LetExpression*) {
  std::abort();
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitBlock(BlockExpression* node) {
  Eval(node->expr_);
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitIndex(IndexExpression*) {
  std::abort();
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitFnCall(FnCallExpression* node) {
  if (node->fn_name_.empty()) {
    FMT_ASSERT(false, "Unimplemented");
  }

  auto symbol = node->layer_->RetrieveSymbol(node->fn_name_);

  if (!symbol) {
    node->layer_->Print();
    throw std::runtime_error{
        fmt::format("Could not find function {} at loc {}",  //
                    node->fn_name_, node->GetLocation().Format())};
  }

  auto ty = symbol->GetType();
  auto ctx = node->layer_;

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

  work_queue_.push_back({
      .tag = TraitTags::CALLABLE,
      .bound = ty,
      .none = {},
      .location = node->GetLocation(),
  });

  PushEqual(node->GetLocation(), f, ty);

  return_value = result_ty;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitIntrinsic(IntrinsicCall* node) {
  for (auto a : node->arguments_) {
    Eval(a);
  }

  switch (node->intrinsic) {
    case ast::elaboration::Intrinsic::PRINT:
      PushEqual(node->GetLocation(), Eval(node->arguments_.at(0)),
                MakeTypePtr(&builtin_char));
      break;

    case ast::elaboration::Intrinsic::ASSERT:
      PushEqual(node->GetLocation(), Eval(node->arguments_.at(0)),
                &builtin_bool);
      break;

    case ast::elaboration::Intrinsic::IS_NULL:
      PushEqual(node->GetLocation(), Eval(node->arguments_.at(0)),
                MakeTypePtr(MakeTypeVar(node->layer_)));
      break;

    default:
      std::abort();
  }

  return_value = node->GetType();
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  node->type_ = MakeTypeVar(node->layer_);

  auto loc = node->GetLocation();

  for (auto& mem : node->initializers_) {
    auto field_type = mem.init ? Eval(mem.init) : &builtin_unit;
    auto trait = MakeHasFieldTrait(node->type_, mem.field, field_type, loc);
    work_queue_.push_back(std::move(trait));
  }

  return_value = node->type_;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitFieldAccess(FieldAccessExpression* node) {
  node->type_ = MakeTypeVar(node->layer_);
  auto str = Eval(node->struct_expression_);

  auto loc = node->GetLocation();
  auto trait = MakeHasFieldTrait(str, node->field_name_, node->type_, loc);
  work_queue_.push_back(std::move(trait));

  return_value = node->type_;
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitVarAccess(VarAccessExpression* node) {
  if (auto symbol = node->layer_->RetrieveSymbol(node->name_)) {
    auto ty = symbol->GetType();

    node->type_ = ty;

    KnownParams p = {};
    return_value = ty->tag == TypeTag::TY_FUN ? Instantinate(ty, p) : ty;

    return;
  }

  throw std::runtime_error{
      fmt::format("Could not find {}", node->name_.GetName())};
}

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitLiteral(LiteralExpression* node) {
  switch (node->token_.type) {
    case lex::TokenType::INTEGER:
      return_value = &builtin_int;
      break;

    case lex::TokenType::STRING:
      return_value = MakeTypePtr(&builtin_char);
      break;

    case lex::TokenType::CHAR:
      return_value = &builtin_char;
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

//////////////////////////////////////////////////////////////////////

void AlgorithmW::VisitTypecast(TypecastExpression* node) {
  auto e = Eval(node->expr_);

  work_queue_.push_back(Trait{
      .tag = TraitTags::CONVERTIBLE_TO,
      .bound = FindLeader(e),
      .convertible_to = {.to_type = FindLeader(node->type_)},
      .location = node->GetLocation(),
  });

  return_value = node->type_;
}

//////////////////////////////////////////////////////////////////////

}  // namespace types::constraints::generate
