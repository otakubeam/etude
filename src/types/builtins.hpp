#pragma once

#include <types/type.hpp>

namespace types {

class BuiltinType : public Type {
 public:
  bool IsEqual(Type* other) override {
    return other->IsEqual(this);
  }

  bool IsEqual(BuiltinType* other) override {
    return other == this;
  }

  bool IsEqual(FnType*) override {
    return false;
  }
};

//////////////////////////////////////////////////////////////////////

// Built-in type singletons
extern BuiltinType  //
    builtin_unit,   //
    builtin_int,    //
    builtin_bool,   //
    builtin_string;

}  // namespace types
