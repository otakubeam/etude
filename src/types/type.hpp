#pragma once

#include <vector>

namespace types {

class BuiltinType;
class StructType;
class FnType;

//////////////////////////////////////////////////////////////////////

class Type {
 public:
  ~Type() = default;

  // To resolve the first type
  virtual bool IsEqual(Type* other) = 0;
  virtual bool DiffersFrom(Type* other) {
    return !IsEqual(other);
  }

  // To resolve the second type
  virtual bool IsEqual(BuiltinType* other) = 0;

  virtual bool IsEqual(StructType* other) = 0;
  virtual bool IsEqual(FnType* other) = 0;

  virtual bool IsBuiltin() {
    return false;
  }

  virtual bool IsStruct() {
    return false;
  }

  virtual bool IsFnType() {
    return false;
  }

  // TODO:
  // virtual bool IsEqual(TypeAlias* other) = 0;
};

//////////////////////////////////////////////////////////////////////

// class ArrayType : public Type {
//  private:
//   Type* underlying_;
// };

//////////////////////////////////////////////////////////////////////
};  // namespace types
