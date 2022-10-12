#pragma once

#include <types/repr/builtins.hpp>

#include <types/type.hpp>

#include <fmt/core.h>

#include <string>

namespace types {

//////////////////////////////////////////////////////////////////////

class PointerType : public Type {
 public:
  PointerType(Type* underlying) : underlying_{underlying} {
  }

  virtual bool IsEqual(Type* other) override {
    return other->IsEqual(this);
  }

  virtual bool IsEqual(PointerType* other) override {
    return other->underlying_->IsEqual(this->underlying_);
  };

  virtual bool IsEqual(BuiltinType* other) override {
    // Allow initializing pointers with unit
    return other == &builtin_unit;
  }

  virtual bool IsEqual(StructType*) override {
    return false;
  }

  virtual bool IsEqual(FnType*) override {
    return false;
  }

  Type* Underlying() {
    return underlying_;
  }

  std::string Format() override {
    return "*" + underlying_->Format();
  }

 private:
  // Perhaps should not work with functions?
  // They are already pointers, after all
  Type* underlying_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

}  // namespace types
