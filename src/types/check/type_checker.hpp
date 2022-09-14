#pragma once

#include <types/check/type_error.hpp>
#include <types/repr/struct_type.hpp>
#include <types/repr/builtins.hpp>
#include <types/repr/fn_type.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <rt/structs/struct_object.hpp>

namespace types::check {

//////////////////////////////////////////////////////////////////////

class TypeChecker : public ReturnVisitor<Type*> {
 public:
  TypeChecker() {
    // Declare intrinsics
    global_type_store.Declare("print", new FnType({}));
  }

  virtual void VisitStatement(Statement*) override {
    FMT_ASSERT(false, "Visiting bare statement");
  }

  virtual void VisitAssignment(AssignmentStatement* node) override {
    // auto type = Eval(node->value_);
    // auto type2 = Eval(node->target_);
    // if (DiffersFrom(type1, type2)) {...}
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement* var_decl) override {
    auto type = Eval(var_decl->value_);
    env_->Declare(var_decl->lvalue_->name_.GetName(), type);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructDecl(StructDeclStatement* node) override {
    auto name = node->name_.GetName();
    struct_decls_.Declare(name, node);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement* fn_decl) override {
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
      if (fn_return_expect->DiffersFrom(Eval(fn_decl->block_))) {
        throw check::FnBlockError{};
      }

      // 3. Restore previous return expect
      fn_return_expect = saved;
    }

    // If everything went good
    env_->Declare(fn_decl->name_.GetName(), declared_type);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitReturn(ReturnStatement* return_stmt) override {
    if (!fn_return_expect) {
      return;  // Program finishes
    }

    if (fn_return_expect->DiffersFrom(Eval(return_stmt->return_value_))) {
      throw check::FnDeclarationError{};
    }
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitYield(YieldStatement*) override {
    // TODO: throw up to closest match
    return_value = &builtin_unit;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement* expr_stmt) override {
    Eval(expr_stmt->expr_);  // Type-check
    return_value = &builtin_unit;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression*) override {
    FMT_ASSERT(false, "Visiting bare expression");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitComparison(ComparisonExpression* cmp_expr) override {
    if (Eval(cmp_expr->left_) != &builtin_int) {
      throw check::ArithCmpError{"left"};
    }

    if (Eval(cmp_expr->right_) != &builtin_int) {
      throw check::ArithCmpError{"right"};
    }

    return_value = &builtin_bool;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBinary(BinaryExpression* bin_expr) override {
    if (Eval(bin_expr->left_) != &builtin_int) {
      throw check::ArithAddError{"left"};
    }

    if (Eval(bin_expr->right_) != &builtin_int) {
      throw check::ArithAddError{"right"};
    }

    return_value = &builtin_int;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitUnary(UnaryExpression* un_expr) override {
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

  virtual void VisitIf(IfExpression* if_expr) override {
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

  virtual void VisitBlock(BlockExpression* block) override {
    Environment<Type*>::ScopeGuard guard{&env_};
    for (auto stmt : block->stmts_) {
      Eval(stmt);
    }

    return_value = block->final_ ? Eval(block->final_)  //
                                 : &builtin_unit;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFnCall(FnCallExpression* fn_call) override {
    // The fact that the fucntion block returns the
    // declared type is checked on declaration place
    Type* stored_type = env_->Get(fn_call->fn_name_.GetName()).value();
    auto fn_type = dynamic_cast<FnType*>(stored_type);

    std::vector<Type*> args_types;
    for (auto arg : fn_call->arguments_) {
      args_types.push_back(Eval(arg));
    }

    FnType inferred_type{std::move(args_types), fn_type->GetReturnType()};

    if (fn_call->fn_name_.GetName() == "print") {
      // Intrinsic: don't type-check
    } else if (fn_type->DiffersFrom(&inferred_type)) {
      throw check::FnInvokeError{fn_call->fn_name_.GetName(), "Unknown",
                                 fn_call->fn_name_.tk_loc.Format()};
    }

    return_value = fn_type->GetReturnType();
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructConstruction(
      StructConstructionExpression* s_cons) override {
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

  virtual void VisitFieldAccess(FieldAccessExpression* node) override {
    node->struct_expression_->Accept(this);

    auto t = node->struct_expression_->GetType();
    FMT_ASSERT(t, "FieldAccessExpression: Typechecker fault");

    auto str_t = dynamic_cast<StructType*>(t);

    auto searching = node->field_name_.GetName();

    if (str_t == nullptr) {
      throw FieldAccessError::NotAStruct("<some-expression> before " +
                                         searching);
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

  virtual void VisitLiteral(LiteralExpression* lit) override {
    switch (lit->token_.type) {
      case lex::TokenType::NUMBER:
        return_value = &builtin_int;
        break;

      case lex::TokenType::STRING:
        return_value = &builtin_string;
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

  virtual void VisitVarAccess(VarAccessExpression* ident) override {
    Type* t = env_->Get(ident->name_.GetName()).value();
    ident->type_ = t;
    return_value = t;
  }

 private:
  Type* fn_return_expect = nullptr;

  ////////////////////////////////////////////////////////////////////

  using TypeStore = Environment<Type*>;
  TypeStore global_type_store = TypeStore::MakeGlobal();

  TypeStore* env_ = &global_type_store;

  ////////////////////////////////////////////////////////////////////

  using DeclStore = Environment<StructDeclStatement*>;
  DeclStore struct_decls_ = DeclStore::MakeGlobal();
};

//////////////////////////////////////////////////////////////////////

}  // namespace types::check
