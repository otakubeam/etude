#pragma once

#include <ast/visitors/template_visitor.hpp>
#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <types/type_error.hpp>
#include <types/builtins.hpp>
#include <types/fn_type.hpp>

class TypeChecker : public EnvVisitor<types::Type*> {
 public:
  TypeChecker() {
    // Declare intrinsics
    env_->Declare("print", new types::FnType({}));
  }

  virtual void VisitStatement(Statement*) override {
    FMT_ASSERT(false, "Visiting bare statement");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement* var_decl) override {
    auto type = Eval(var_decl->value_);
    env_->Declare(var_decl->lvalue_->name_.GetName(), type);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructDecl(StructDeclStatement*) override {
    std::abort();
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement* fn_decl) override {
    auto declared_type = fn_decl->type_;

    {
      Environment<types::Type*>::ScopeGuard guard{&env_};

      // Declare the function itself (to enable checking recursive fns)

      env_->Declare(fn_decl->name_.GetName(), declared_type);

      // Declare all the parameters of a function

      for (auto fm : fn_decl->formals_) {
        env_->Declare(fm.ident.GetName(), fm.type);
      }

      auto saved = fn_return_expect;
      fn_return_expect = declared_type->GetReturnType();

      // TODO: Allow for final return?
      if (Eval(fn_decl->block_) != fn_return_expect) {
        throw types::TypeError{
            .msg = fmt::format(
                "Return type does not match the function declaration")};
      }

      fn_return_expect = saved;
    }

    // If everything went good.
    env_->Declare(fn_decl->name_.GetName(), declared_type);
  }

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    rt::SBObject value;
  };

  virtual void VisitReturn(ReturnStatement* return_stmt) override {
    if (!fn_return_expect) {
      return;  // Program finishes
    }

    if (fn_return_expect != Eval(return_stmt->return_value_)) {
      throw types::TypeError{
          .msg = fmt::format(
              "Return type does not match the function declaration")};
    }
  }

  ////////////////////////////////////////////////////////////////////

  struct YieldedValue {
    rt::SBObject value;
  };

  virtual void VisitYield(YieldStatement*) override {
    // TODO: throw up to closest match
    return_value = &types::builtin_unit;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement* expr_stmt) override {
    Eval(expr_stmt->expr_);  // Type-check
    return_value = &types::builtin_unit;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression*) override {
    FMT_ASSERT(false, "Visiting bare expression");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitComparison(ComparisonExpression* cmp_expr) override {
    if (Eval(cmp_expr->left_) != &types::builtin_int) {
      throw types::TypeError{fmt::format(
          "Comparison expression at {} has non-int left operand", "Unknown")};
    }

    if (Eval(cmp_expr->right_) != &types::builtin_int) {
      throw types::TypeError{fmt::format(
          "Comparison expression at {} has non-int right operand", "Unknown")};
    }

    return_value = &types::builtin_bool;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBinary(BinaryExpression* bin_expr) override {
    if (Eval(bin_expr->left_) != &types::builtin_int) {
      throw types::TypeError{fmt::format(
          "Binary expression at {} has non-int left operand", "Unknown")};
    }

    if (Eval(bin_expr->right_) != &types::builtin_int) {
      throw types::TypeError{fmt::format(
          "Binary expression at {} has non-int right operand", "Unknown")};
    }

    return_value = &types::builtin_int;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitUnary(UnaryExpression* un_expr) override {
    switch (un_expr->operator_.type) {
      case lex::TokenType::NOT:
        if (Eval(un_expr->operand_) != &types::builtin_bool) {
          throw types::TypeError{"Negating non-bool"};
        } else {
          return_value = &types::builtin_bool;
          return;
        }

      case lex::TokenType::MINUS:
        if (Eval(un_expr->operand_) != &types::builtin_int) {
          throw types::TypeError{"Negating non-int"};
        } else {
          return_value = &types::builtin_int;
          return;
        }

      default:
        FMT_ASSERT(false, "Unreachable");
    }
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitIf(IfExpression* if_expr) override {
    if (!types::builtin_bool.IsEqual(Eval(if_expr->condition_))) {
      throw types::TypeError{fmt::format(
          "If expression at {} has non-bool condition",
          "Unknown "  //
          )};
    }

    auto true_type = Eval(if_expr->true_branch_);

    auto false_type = Eval(if_expr->false_branch_);

    if (!true_type->IsEqual(false_type)) {
      throw types::TypeError{fmt::format(
          "If expression at {} has arms of different types",
          "Unknown "  //
          )};
    }

    return_value = true_type;
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBlock(BlockExpression* block) override {
    {
      Environment<types::Type*>::ScopeGuard guard{&env_};
      for (auto stmt : block->stmts_) {
        Eval(stmt);
      }

      return_value = block->final_ ? Eval(block->final_)  //
                                   : &types::builtin_unit;
    }
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFnCall(FnCallExpression* fn_call) override {
    // The fact that the fucntion block returns the
    // declared type is checked on declaration place
    types::Type* stored_type = env_->Get(fn_call->fn_name_.GetName()).value();
    auto fn_type = dynamic_cast<types::FnType*>(stored_type);

    std::vector<types::Type*> args_types;
    for (auto arg : fn_call->arguments_) {
      args_types.push_back(Eval(arg));
    }

    types::FnType inferred_type{std::move(args_types),
                                fn_type->GetReturnType()};

    if (fn_call->fn_name_.GetName() == "print") {
      // Intrinsic: don't type-check
    } else if (!fn_type->IsEqual(&inferred_type)) {
      throw types::TypeError{fmt::format(
          "Function {} at {} and its invocation at {} do not correspond",
          fn_call->fn_name_.GetName(),  //
          "Unknown ",                   //
          fn_call->fn_name_.start.Format())};
    }

    return_value = fn_type->GetReturnType();
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructConstruction(StructConstructionExpression*) override {
    std::abort();
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitLiteral(LiteralExpression* lit) override {
    switch (lit->token_.type) {
      case lex::TokenType::NUMBER:
        return_value = &types::builtin_int;
        break;

      case lex::TokenType::STRING:
        return_value = &types::builtin_string;
        break;

      case lex::TokenType::TRUE:
      case lex::TokenType::FALSE:
        return_value = &types::builtin_bool;
        break;

      default:
        FMT_ASSERT(false, "Typechecking unknown literal");
    }
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitLvalue(VarAccessExpression* ident) override {
    types::Type* t = env_->Get(ident->name_.GetName()).value();
    return_value = t;
  }

 private:
  types::Type* fn_return_expect = nullptr;
};
