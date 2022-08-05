#pragma once

#include <rt/functions/function.hpp>

#include <ast/statements.hpp>

namespace rt {

//////////////////////////////////////////////////////////////////////

struct FunctionObject : public IFunction {
  FunctionObject(FunDeclStatement* fn) : fn{fn} {
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

inline bool operator==(FunctionObject lhs, FunctionObject rhs) {
  return lhs.fn == rhs.fn;
}

//////////////////////////////////////////////////////////////////////

}  // namespace rt
