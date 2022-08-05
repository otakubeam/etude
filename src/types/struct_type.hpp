#pragma once

#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

class StructType : public Type {
 public:
  virtual bool IsEqual(Type* other) {
    return other->IsEqual(this);
  }

  virtual bool IsEqual(BuiltinType*) {
    return false;
  }

  virtual bool IsEqual(StructType* other) {
    if (other->types_.size() != types_.size()) {
      return false;
    }

    for (std::size_t i = 0; i < types_.size(); i++) {
      if (!types_[i]->IsEqual(other->types_[i])) {
        return false;
      }
    }

    return true;
  }

  virtual bool IsEqual(FnType*) {
    return false;
  }

 private:
  std::vector<Type*> types_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace types
