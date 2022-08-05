#pragma once

#include <ast/visitors/template_visitor.hpp>
#include <ast/scope/environment.hpp>
#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <rt/functions/native_function.hpp>
#include <rt/base_object.hpp>

class Evaluator : public EnvVisitor<rt::SBObject> {
 public:
  friend struct IFunction;

  Evaluator();
  virtual ~Evaluator();

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStatement(Statement* /* node */) override {
    FMT_ASSERT(false, "Visiting bare statement");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement* node) override {
    auto name = node->lvalue_->name_.GetName();
    auto val = Eval(node->value_);
    env_->Declare(name, val);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement* node) override {
    auto name = node->name_.GetName();
    rt::SBObject val = {new rt::FunctionObject{node}};
    env_->Declare(name, val);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructDecl(StructDeclStatement*) override {
    std::abort();
  }

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    rt::SBObject value;
  };

  virtual void VisitReturn(ReturnStatement* node) override {
    auto ret = node->return_value_ ?  //
                   Eval(node->return_value_)
                                   : rt::SBObject{};
    throw ReturnedValue{ret};
  }

  ////////////////////////////////////////////////////////////////////

  struct YieldedValue {
    rt::SBObject value;
  };

  virtual void VisitYield(YieldStatement* node) override {
    auto ret = node->yield_value_ ?  //
                   Eval(node->yield_value_)
                                  : rt::SBObject{};
    throw YieldedValue{ret};
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement* node) override {
    Eval(node->expr_);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression* node) override;

  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression* node) override;

  ////////////////////////////////////////////////////////////////////

  virtual void VisitIf(IfExpression* node) override {
    auto cond = GetPrim<bool>(Eval(node->condition_));
    auto* branch = cond ? node->true_branch_ : node->false_branch_;
    return_value = Eval(branch);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBlock(BlockExpression* node) override {
    Environment<rt::SBObject>::ScopeGuard guard{&env_};

    try {
      for (auto stmt : node->stmts_) {
        Eval(stmt);
      }
    } catch (YieldedValue yield) {
      return_value = yield.value;
      return;
    }

    return_value = node->final_ ? Eval(node->final_) : rt::SBObject{};
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStructConstruction(StructConstructionExpression*) override {
    std::abort();
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* lit) override;
  virtual void VisitLvalue(LvalueExpression* ident) override;

  ////////////////////////////////////////////////////////////////////
};
