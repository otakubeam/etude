#pragma once

#include <ast/visitors/visitor.hpp>
#include <ast/syntax_tree.hpp>

#include <fmt/core.h>

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
