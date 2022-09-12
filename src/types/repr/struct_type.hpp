#pragma once

#include <types/type.hpp>

#include <fmt/core.h>

#include <string>

namespace types {

//////////////////////////////////////////////////////////////////////

class StructType : public Type {
 public:
  StructType(std::string name) : name_{name} {
  }

  virtual bool IsEqual(Type* other) override {
    return other->IsEqual(this);
  }

  virtual bool IsEqual(BuiltinType*) override {
    return false;
  }

  virtual bool IsEqual(StructType* other) override {
    // XXX: Radically simplify!
    return other->name_ == name_;

    // if (other->types_.size() != types_.size()) {
    //   return false;
    // }
    //
    // for (std::size_t i = 0; i < types_.size(); i++) {
    //   if (!types_[i]->IsEqual(other->types_[i])) {
    //     return false;
    //   }
    // }
    //
    // return true;
  }

  virtual bool IsEqual(FnType*) override {
    return false;
  }

  std::string GetName() {
    return name_;
  }

  virtual bool IsStruct() override {
    return true;
  }

 private:
  std::string name_;
  // std::vector<Type*> types_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace types
