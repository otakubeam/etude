#pragma once

#include <ast/visitors/template_visitor.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <rt/scope/environment.hpp>
#include <rt/native_function.hpp>
#include <rt/primitive_type.hpp>
#include <rt/base_object.hpp>

class Evaluator : public EnvVisitor<SBObject> {
 public:
  friend struct IFunction;

  Evaluator();
  virtual ~Evaluator();

  ////////////////////////////////////////////////////////////////////

  virtual void VisitStatement(Statement* /* node */) override {
    FMT_ASSERT(false, "Visiting bare statement");
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitIf(IfStatement* node) override {
    auto cond = GetPrim<bool>(Eval(node->condition_));
    auto* branch = cond ? node->true_branch_ : node->false_branch_;
    Eval(branch);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitVarDecl(VarDeclStatement* node) override {
    auto name = node->lvalue_->token_.GetName();
    auto val = Eval(node->value_);
    env_->Declare(name, val);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitFunDecl(FunDeclStatement* node) override {
    auto name = node->name_.GetName();
    SBObject val = {new FunctionType{node}};
    env_->Declare(name, val);
  }

  ////////////////////////////////////////////////////////////////////

  struct ReturnedValue {
    SBObject value;
  };

  virtual void VisitReturn(ReturnStatement* node) override {
    auto ret = node->return_value_ ?  //
                   Eval(node->return_value_)
                                   : SBObject{};
    throw ReturnedValue{ret};
  }

  ////////////////////////////////////////////////////////////////////

  struct YieldedValue {
    SBObject value;
  };

  virtual void VisitYield(YieldStatement* node) override {
    auto ret = node->yield_value_ ?  //
                   Eval(node->yield_value_)
                                  : SBObject{};
    throw YieldedValue{ret};
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExprStatement(ExprStatement* node) override {
    Eval(node->expr_);
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitBlockStatement(BlockStatement* node) override {
    Environment::ScopeGuard guard{&env_};

    // TODO: make an expression

    // try {
    //   for (auto stmt : node->stmts_) {
    //     Eval(stmt);
    //   }
    // } catch (YieldedValue yield) {
    //
    // }

    for (auto stmt : node->stmts_) {
      Eval(stmt);
    }
  }

  ////////////////////////////////////////////////////////////////////

  virtual void VisitExpression(Expression* node) override;

  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression* node) override;
  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* lit) override;

 private:
  // TODO: Translation from SemInfo to AST nodes
  // should have happened during parsing
  PrimitiveType FromSemInfo(lex::Token::SemInfo sem_info) {
    switch (sem_info.index()) {
        // std::monostate
      case 0:
        throw "Error: evaluating monostate literal";

        // std::string
      case 1:
        return PrimitiveType{std::get<std::string>(sem_info)};

        // bool
      case 2:
        FMT_ASSERT(false, "\n Error: Unreachable \n");

        // int
      case 3:
        return PrimitiveType{std::get<int>(sem_info)};

      default:
        FMT_ASSERT(false, "\n Error: Non-exhaustive match \n");
    }
  }
};
