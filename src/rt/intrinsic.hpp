#pragma once

#include <rt/ifunction.hpp>

#include <fmt/core.h>

namespace rt {

//////////////////////////////////////////////////////////////////////

struct Print : public IFunction {
  virtual SBObject Compute(EnvVisitor<SBObject>*,
                           std::vector<SBObject> args) override {
    for (auto obj : args) {
      fmt::print("{} ", Format(obj));
    }

    fmt::print("\n");
    return {};
  };
};

//////////////////////////////////////////////////////////////////////

}
