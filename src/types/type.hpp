#pragma once

namespace types {

class Type {
 public:
  ~Type() = default;

  virtual bool IsEqual(Type* other) = 0;
};

//////////////////////////////////////////////////////////////////////

class BuiltinType : public Type {
 public:
  bool IsEqual(Type* other) override {
    return other == this;
  }
};

// Built-in type singletons
extern BuiltinType  //
    builtin_int,    //
    builtin_bool,   //
    builtin_string;

//////////////////////////////////////////////////////////////////////

class ArrayType : public Type {
 private:
  Type* underlying_;
};

//////////////////////////////////////////////////////////////////////

class StructType : public Type {
 private:
  Type* types_;
};

//////////////////////////////////////////////////////////////////////

};  // namespace types
