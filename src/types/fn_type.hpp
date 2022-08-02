#pragma once

#include <types/builtins.hpp>

#include <types/type.hpp>

namespace types {

class FnType : public Type {
 public:
  bool IsEqual(Type* other) override {
    return other->IsEqual(this);
  }

  bool IsEqual(BuiltinType*) override {
    // Certainly false
    return false;
  }

  bool IsEqual(FnType*) override {
    // Mb true
    return true;
  }

 private:
  std::vector<Type*> arg_types_;
  Type* return_type_{&builtin_unit};
};

}  // namespace types
