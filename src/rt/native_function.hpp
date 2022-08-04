#pragma once

#include <ast/statements.hpp>

#include <rt/ifunction.hpp>

#include <algorithm>
#include <ranges>

namespace rt {

//////////////////////////////////////////////////////////////////////

struct FunctionType : public IFunction {
  FunctionType(FunDeclStatement* fn) : fn{fn} {
  }

  virtual SBObject Compute(EnvVisitor<SBObject>* e,
                           std::vector<SBObject> args) override {
    Environment<SBObject>::ScopeGuard(&e->env_);

    for (size_t i = 0; i < args.size(); i++) {
      auto name = fn->formals_[i].ident.GetName();
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

}  // namespace rt
