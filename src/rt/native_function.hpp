#pragma once

#include <ast/statements.hpp>

#include <rt/ifunction.hpp>

#include <algorithm>
#include <ranges>

//////////////////////////////////////////////////////////////////////

struct FunctionType : public IFunction {
  FunctionType(FunDeclStatement* fn) : fn{fn} {
  }

  virtual SBObject Compute(EnvVisitor<SBObject>* e,
                           std::vector<SBObject> args) override {
    if (args.size() != fn->formals_.size()) {
      throw "Bad function call:" //
         "args and params size do not correspond";
    }

    Environment::ScopeGuard(&e->env_);

    for (size_t i = 0; i < args.size(); i++) {
      auto name = fn->formals_[i].GetName();
      e->env_->Declare(name, args[i]);
    }

    return {e->Eval(fn->block_)};
  };

  FunDeclStatement* fn;
};

inline bool operator==(FunctionType lhs, FunctionType rhs) {
  return lhs.fn == rhs.fn;
}

//////////////////////////////////////////////////////////////////////
