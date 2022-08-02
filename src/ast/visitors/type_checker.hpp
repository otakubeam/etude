#pragma once

#include <ast/visitors/template_visitor.hpp>
#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <types/type_error.hpp>
#include <types/builtins.hpp>
#include <types/fn_type.hpp>

class TypeChecker : public EnvVisitor<types::Type*> {
 public:
  virtual void VisitStatement(Statement*) override {
    FMT_ASSERT(false, "Visiting bare statement");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement*) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement*) override {
  }

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    SBObject value;
  };

  virtual void VisitReturn(ReturnStatement*) override {
  }

  ////////////////////////////////////////////////////////////////////

  struct YieldedValue {
    SBObject value;
  };

  virtual void VisitYield(YieldStatement*) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement*) override {
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression*) override {
    FMT_ASSERT(false, "Visiting bare expression");
  }

  virtual void VisitComparison(ComparisonExpression*) override {
    std::abort();
  }

  virtual void VisitBinary(BinaryExpression*) override {
    std::abort();
  }

  virtual void VisitUnary(UnaryExpression*) override {
    std::abort();
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
    for (auto stmt : block->stmts_) {
      stmt->Accept(this);
    }

    return_value = block->final_ ? Eval(block->final_)  //
                                 : &types::builtin_unit;
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

    if (!fn_type->IsEqual(&inferred_type)) {
      throw types::TypeError{fmt::format(
          "Function {} at {} and its Invocation at {} do not correspond",
          fn_call->fn_name_.GetName(),  //
          "Unknown ",                   //
          fn_call->fn_name_.start.Format())};
    }

    return_value = fn_call->type_;
  }

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

  virtual void VisitLvalue(LvalueExpression* ident) override {
    types::Type* t = env_->Get(ident->name_.GetName()).value();
    return_value = t;
  }
};
