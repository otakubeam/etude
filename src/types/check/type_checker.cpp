#include <types/check/type_checker.hpp>

#include <types/check/type_error.hpp>

#include <ast/statements.hpp>

namespace types::check {

TypeChecker::TypeChecker() {
  // Declare intrinsics
  global_type_store.Declare("print", new FnType({}));
  global_type_store.Declare("assert", new FnType({}));
  global_type_store.Declare("isNull", new FnType({}, &builtin_bool));
}

TypeChecker::~TypeChecker() = default;

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitDeref(DereferenceExpression* node) {
  auto ptr_type = Eval(node->operand_);

  if (auto type = dynamic_cast<types::PointerType*>(ptr_type)) {
    // The type of *(expr : *T) is T
    node->type_ = type->Underlying();
    return_value = node->type_;
  } else {
    throw DereferenceError{node->GetLocation()};
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
  auto target_type = Eval(node->target_);
  auto value_type = Eval(node->value_);

  if (value_type->DiffersFrom(target_type)) {
    throw check::AssignmentError{node->GetLocation()};
  }
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitVarDecl(VarDeclStatement* node) {
  auto type = Eval(node->value_);
  variable_type_store_->Declare(node->GetVarName(), type);
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitStructDecl(StructDeclStatement* node) {
  auto name = node->GetStructName();
  struct_decls_.Declare(name, node);
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitFunDecl(FunDeclStatement* node) {
  auto declared_type = node->type_;

  {
    Environment<Type*>::ScopeGuard guard{&variable_type_store_};

    // Declare the function itself (to enable checking recursive fns)

    variable_type_store_->Declare(node->GetFunctionName(), declared_type);

    // Declare all the parameters of a function

    for (auto formal : node->formals_) {
      variable_type_store_->Declare(formal.GetParameterName(), formal.type);
    }

    // A little dance to handle nesting of functions:
    // 1. Save previous return_expect
    auto saved = fn_return_expect;
    fn_return_expect = declared_type->GetReturnType();

    // 2. Do the check

    auto inferred_return_type = Eval(node->block_);

    if (fn_return_expect->DiffersFrom(inferred_return_type) &&
        inferred_return_type != &builtin_unit) {
      throw check::FnBlockFinalError{node->GetLocation()};
    }

    // 3. Restore previous return expect
    fn_return_expect = saved;
  }

  // If everything went good
  variable_type_store_->Declare(node->GetFunctionName(), declared_type);
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitReturn(ReturnStatement* node) {
  if (!fn_return_expect) {
    return;  // Program finishes
  }

  if (fn_return_expect->DiffersFrom(Eval(node->return_value_))) {
    throw check::FnReturnStatementError{node->GetLocation()};
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

void TypeChecker::VisitComparison(ComparisonExpression* node) {
  if (Eval(node->left_) != &builtin_int) {
    throw check::ArithCmpError{node->GetLocation(), "left"};
  }

  if (Eval(node->right_) != &builtin_int) {
    throw check::ArithCmpError{node->GetLocation(), "right"};
  }

  return_value = &builtin_bool;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitBinary(BinaryExpression* node) {
  if (Eval(node->left_) != &builtin_int) {
    throw check::ArithAddError{node->GetLocation(), "left"};
  }

  if (Eval(node->right_) != &builtin_int) {
    throw check::ArithAddError{node->GetLocation(), "right"};
  }

  return_value = &builtin_int;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitUnary(UnaryExpression* node) {
  Type* expected_type = nullptr;
  auto operand_type = Eval(node->operand_);

  switch (node->operator_.type) {
    case lex::TokenType::NOT:
      expected_type = &builtin_bool;
      break;

    case lex::TokenType::MINUS:
      expected_type = &builtin_int;
      break;

    default:
      FMT_ASSERT(false, "Unreachable");
  }

  if (expected_type->DiffersFrom(operand_type)) {
    throw ArithNegateError{node->GetLocation()};
  }

  return_value = expected_type;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitIf(IfExpression* if_expr) {
  if (builtin_bool.DiffersFrom(Eval(if_expr->condition_))) {
    throw check::IfCondError{if_expr->GetLocation()};
  }

  auto true_type = Eval(if_expr->true_branch_);
  auto false_type = if_expr->false_branch_ ? Eval(if_expr->false_branch_)  //
                                           : &builtin_unit;
  if (true_type->DiffersFrom(false_type)) {
    throw check::IfArmsError{if_expr->GetLocation()};
  }

  return_value = true_type;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitBlock(BlockExpression* node) {
  Environment<Type*>::ScopeGuard guard{&variable_type_store_};

  for (auto stmt : node->stmts_) {
    Eval(stmt);
  }

  return_value = node->final_ ? Eval(node->final_)  //
                              : &builtin_unit;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitFnCall(FnCallExpression* node) {
  // The fact that the fucntion block returns the
  // declared type is checked on declaration place

  std::vector<Type*> arg_types;
  for (auto argument : node->arguments_) {
    arg_types.push_back(Eval(argument));
  }

  auto name = node->GetFunctionName();

  // Intrinsic: don't type-check

  // TODO: it seems I need to pull the native_table here as well
  // So the table must be generic over all backends
  // But the concrete implementations need to be provided separately

  if (name == "print" || name == "isNull" || name == "assert") {
    return_value = name == "isNull" ? &builtin_bool : &builtin_unit;
    node->is_native_call_ = true;
    return;
  }

  // Normal function

  // TODO: catch errors
  Type* stored_type = variable_type_store_->Get(name).value();
  auto fn_type = dynamic_cast<FnType*>(stored_type);

  FnType inferred_type{std::move(arg_types), fn_type->GetReturnType()};

  if (fn_type->DiffersFrom(&inferred_type)) {
    throw check::FnInvokeError{node->GetFunctionName(), node->GetLocation()};
  }

  return_value = fn_type->GetReturnType();
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitStructConstruction(StructConstructionExpression* node) {
  auto struct_declaration = struct_decls_.Get(node->GetStructName()).value();

  for (size_t i = 0; i < struct_declaration->field_names_.size(); i++) {
    auto field_formal_type = struct_declaration->field_types_[i];
    auto field_passed_type = Eval(node->values_[i]);

    if (field_formal_type->DiffersFrom(field_passed_type)) {
      throw check::StructInitializationError{node->GetLocation()};
    }
  }

  return_value = struct_declaration->type_;
  node->type_ = return_value;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitFieldAccess(FieldAccessExpression* node) {
  node->struct_expression_->Accept(this);

  auto type = node->struct_expression_->GetType();
  FMT_ASSERT(type, "FieldAccessExpression: Typechecker fault");

  auto struct_type = dynamic_cast<StructType*>(type);

  if (struct_type == nullptr) {
    throw FieldAccessError::NotAStruct(
        node->GetLocation(),
        "<some-expression> before " + node->GetFieldName());
  }

  auto struct_declaration = struct_decls_.Get(struct_type->GetName()).value();
  auto searching = node->GetFieldName();

  for (size_t i = 0; i < struct_declaration->field_names_.size(); i++) {
    if (searching == struct_declaration->field_names_[i].GetName()) {
      return_value = struct_declaration->field_types_[i];
      node->type_ = return_value;
      return;
    }
  }

  throw check::FieldAccessError{node->GetLocation(), searching,
                                struct_type->GetName()};
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitLiteral(LiteralExpression* node) {
  switch (node->token_.type) {
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

  node->type_ = return_value;
}

////////////////////////////////////////////////////////////////////

void TypeChecker::VisitVarAccess(VarAccessExpression* node) {
  if (auto type = variable_type_store_->Get(node->GetName())) {
    node->type_ = *type;
    return_value = *type;
  } else {
    throw VarAccessError{node->GetLocation()};
  }
}

}  // namespace types::check
