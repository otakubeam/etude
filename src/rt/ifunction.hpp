#pragma once

#include <ast/visitors/template_visitor.hpp>

#include <rt/base_object.hpp>

#include <vector>

namespace rt {

struct IFunction {
  virtual ~IFunction() = default;

  virtual SBObject Compute(EnvVisitor<SBObject>* e,  //
                           std::vector<SBObject>) = 0;
};

}
