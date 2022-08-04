#pragma once

#include <ast/scope/environment.hpp>

#include <ast/visitors/visitor.hpp>
#include <ast/syntax_tree.hpp>

//////////////////////////////////////////////////////////////////////

template <typename T>
class ReturnVisitor : public Visitor {
 public:
  T Eval(TreeNode* expr) {
    FMT_ASSERT(expr,
               "\nError: "
               "Evaluating null expression \n");
    expr->Accept(this);
    return return_value;
  }

 protected:
  T return_value;
};

//////////////////////////////////////////////////////////////////////

template <typename T>
class EnvVisitor : public ReturnVisitor<T> {
 public:
  EnvVisitor() : global_environment(Environment<T>::MakeGlobal()) {
    env_ = &global_environment;
  }

  ~EnvVisitor() = default;

  Environment<T> global_environment;
  Environment<T>* env_{nullptr};
};

//////////////////////////////////////////////////////////////////////
