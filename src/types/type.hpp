#pragma once

#include <vector>

namespace types {

class BuiltinType;
class FnType;

//////////////////////////////////////////////////////////////////////

class Type {
 public:
  ~Type() = default;

  // To resolve the first type
  virtual bool IsEqual(Type* other) = 0;

  // To resolve the second type
  virtual bool IsEqual(BuiltinType* other) = 0;
  virtual bool IsEqual(FnType* other) = 0;
};

//////////////////////////////////////////////////////////////////////

// class ArrayType : public Type {
//  private:
//   Type* underlying_;
// };

//////////////////////////////////////////////////////////////////////

// class StructType : public Type {
//  private:
//   Type* types_;
// };

//////////////////////////////////////////////////////////////////////

};  // namespace types
