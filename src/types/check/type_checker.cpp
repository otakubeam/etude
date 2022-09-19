#include <types/check/type_checker.hpp>

#include <types/check/type_error.hpp>

#include <ast/statements.hpp>

namespace types::check {

TypeChecker::TypeChecker() {
  // Declare intrinsics
  global_type_store.Declare("print", new FnType({}));
  global_type_store.Declare("assert", new FnType({}));
  global_type_store.Declare("isNull", new FnType({}));
}

TypeChecker::~TypeChecker() = default;

void TypeChecker::VisitStatement(Statement*) {
  FMT_ASSERT(false, "Visiting bare statement");
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitDeref(DereferenceExpression* node) {
  auto ptr_type = Eval(node->operand_);

  if (auto type = dynamic_cast<types::PointerType*>(ptr_type)) {
    // The type of *(expr : *T) is T
    node->type_ = type->Underlying();
    return_value = node->type_;
  } else {
    throw DereferenceError{};
  }
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitAddressof(AddressofExpression* node) {
  auto type = Eval(node->operand_);
  node->type_ = new types::PointerType(type);
  return_value = node->type_;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitAssignment(AssignmentStatement* node) {
  auto type1 = Eval(node->value_);
  auto type2 = Eval(node->target_);
  if (type1->DiffersFrom(type2)) {
    throw "error";
  }
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitVarDecl(VarDeclStatement* var_decl) {
  auto type = Eval(var_decl->value_);
  env_->Declare(var_decl->lvalue_->name_.GetName(), type);
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitStructDecl(StructDeclStatement* node) {
  auto name = node->name_.GetName();
  struct_decls_.Declare(name, node);
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitFunDecl(FunDeclStatement* fn_decl) {
  auto declared_type = fn_decl->type_;

  {
    Environment<Type*>::ScopeGuard guard{&env_};

    // Declare the function itself (to enable checking recursive fns)

    env_->Declare(fn_decl->name_.GetName(), declared_type);

    // Declare all the parameters of a function

    for (auto fm : fn_decl->formals_) {
      env_->Declare(fm.ident.GetName(), fm.type);
    }

    // A little dance to handle nesting of functions:
    // 1. Save previous return_expect
    auto saved = fn_return_expect;
    fn_return_expect = declared_type->GetReturnType();

    // 2. Do the check

    auto inferred_ret = Eval(fn_decl->block_);
    if (fn_return_expect->DiffersFrom(inferred_ret) &&
        inferred_ret != &builtin_unit) {
      throw check::FnBlockError{};
    }

    // 3. Restore previous return expect
    fn_return_expect = saved;
  }

  // If everything went good
  env_->Declare(fn_decl->name_.GetName(), declared_type);
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitReturn(ReturnStatement* return_stmt) {
  if (!fn_return_expect) {
    return;  // Program finishes
  }

  if (fn_return_expect->DiffersFrom(Eval(return_stmt->return_value_))) {
    throw check::FnDeclarationError{};
  }
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitYield(YieldStatement*) {
  // TODO: throw up to closest match
  return_value = &builtin_unit;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitExprStatement(ExprStatement* expr_stmt) {
  Eval(expr_stmt->expr_);  // Type-check
  return_value = &builtin_unit;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitExpression(Expression*) {
  FMT_ASSERT(false, "Visiting bare expression");
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitComparison(ComparisonExpression* cmp_expr) {
  if (Eval(cmp_expr->left_) != &builtin_int) {
    throw check::ArithCmpError{"left"};
  }

  if (Eval(cmp_expr->right_) != &builtin_int) {
    throw check::ArithCmpError{"right"};
  }

  return_value = &builtin_bool;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitBinary(BinaryExpression* bin_expr) {
  if (Eval(bin_expr->left_) != &builtin_int) {
    throw check::ArithAddError{"left"};
  }

  if (Eval(bin_expr->right_) != &builtin_int) {
    throw check::ArithAddError{"right"};
  }

  return_value = &builtin_int;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitUnary(UnaryExpression* un_expr) {
  auto operand_type = Eval(un_expr->operand_);

  switch (un_expr->operator_.type) {
    case lex::TokenType::NOT:
      if (builtin_bool.DiffersFrom(operand_type)) {
        throw check::TypeError{"Negating non-bool"};
      }

      return_value = &builtin_bool;
      break;

    case lex::TokenType::MINUS:
      if (builtin_int.DiffersFrom(operand_type)) {
        throw check::TypeError{"Negating non-int"};
      }

      return_value = &builtin_int;
      break;

    default:
      FMT_ASSERT(false, "Unreachable");
  }
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitIf(IfExpression* if_expr) {
  if (builtin_bool.DiffersFrom(Eval(if_expr->condition_))) {
    throw check::IfCondError{"<unknown-loc>"};
  }

  auto true_type = Eval(if_expr->true_branch_);
  auto false_type = if_expr->false_branch_ ? Eval(if_expr->false_branch_)  //
                                           : &builtin_unit;
  if (true_type->DiffersFrom(false_type)) {
    throw check::IfArmsError{"<unknown-loc>"};
  }

  return_value = true_type;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitBlock(BlockExpression* block) {
  Environment<Type*>::ScopeGuard guard{&env_};
  for (auto stmt : block->stmts_) {
    Eval(stmt);
  }

  return_value = block->final_ ? Eval(block->final_)  //
                               : &builtin_unit;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitFnCall(FnCallExpression* fn_call) {
  // The fact that the fucntion block returns the
  // declared type is checked on declaration place

  std::vector<Type*> args_types;
  for (auto arg : fn_call->arguments_) {
    args_types.push_back(Eval(arg));
  }

  auto name = fn_call->fn_name_.GetName();

  // Intrinsic: don't type-check

  if (name == "print" || name == "isNull") {
    return_value = &builtin_bool;
    fn_call->is_native_call_ = true;
    return;
  }

  // Normal function

  Type* stored_type = env_->Get(name).value();
  auto fn_type = dynamic_cast<FnType*>(stored_type);

  FnType inferred_type{std::move(args_types), fn_type->GetReturnType()};

  if (fn_type->DiffersFrom(&inferred_type)) {
    throw check::FnInvokeError{fn_call->fn_name_.GetName(), "Unknown",
                               fn_call->fn_name_.tk_loc.Format()};
  }

  return_value = fn_type->GetReturnType();
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitStructConstruction(
    StructConstructionExpression* s_cons) {
  auto s_decl = struct_decls_.Get(s_cons->struct_name_.GetName()).value();

  for (size_t i = 0; i < s_decl->field_names_.size(); i++) {
    auto field_decl_type = s_decl->field_types_[i];

    auto field_initializer_type = Eval(s_cons->values_[i]);

    if (field_decl_type->DiffersFrom(field_initializer_type)) {
      throw check::StructInitializationError{};
    }
  }

  return_value = s_decl->type_;
  s_cons->type_ = return_value;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitFieldAccess(FieldAccessExpression* node) {
  node->struct_expression_->Accept(this);

  auto t = node->struct_expression_->GetType();
  FMT_ASSERT(t, "FieldAccessExpression: Typechecker fault");

  auto str_t = dynamic_cast<StructType*>(t);

  auto searching = node->field_name_.GetName();

  if (str_t == nullptr) {
    throw FieldAccessError::NotAStruct("<some-expression> before " + searching);
  }

  auto s_decl = struct_decls_.Get(str_t->GetName()).value();

  for (size_t i = 0; i < s_decl->field_names_.size(); i++) {
    if (searching == s_decl->field_names_[i].GetName()) {
      return_value = s_decl->field_types_[i];
      node->type_ = return_value;
      return;
    }
  }

  throw check::FieldAccessError{searching, node->struct_name_.GetName()};
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitLiteral(LiteralExpression* lit) {
  switch (lit->token_.type) {
    case lex::TokenType::NUMBER:
      return_value = &builtin_int;
      break;

    case lex::TokenType::STRING:
      return_value = &builtin_string;
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

  lit->type_ = return_value;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitVarAccess(VarAccessExpression* ident) {
  Type* t = env_->Get(ident->name_.GetName()).value();
  ident->type_ = t;
  return_value = t;
}

}  // namespace types::check
