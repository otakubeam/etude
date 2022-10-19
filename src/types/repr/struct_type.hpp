#pragma once

#include <types/type.hpp>

#include <fmt/core.h>

#include <string>

namespace types {

//////////////////////////////////////////////////////////////////////

class StructType : public Type {
 public:
  struct Member {
    std::string_view name;
    types::Type* type;
  };

  StructType(std::string_view name) : struct_name_{name} {
  }

  StructType(std::string_view name, std::vector<Member> members)
      : struct_name_{name}, members_{std::move(members)} {
  }

  virtual bool IsEqual(Type* other) override {
    return other->IsEqual(this);
  }

  bool IsEqual(PointerType*) override {
    return false;
  }

  bool IsEqual(BuiltinType*) override {
    return false;
  }

  virtual bool IsEqual(StructType* other) override {
    // XXX: Radically simplify!
    return other->struct_name_ == struct_name_;

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

  std::string_view GetName() {
    return struct_name_;
  }

  // Logically, type is not responsible for calculating the offsets of field
  // (this might differ in an interpreter and in a compiler for example)
  // so just send back all the types and let the caller calculate the offset
  std::vector<Type*> AllTypesBeforeField(std::string_view field_name) {
    std::vector<Type*> result;

    for (auto m : members_) {
      if (m.name == field_name) {
        return result;
      }
      result.push_back(m.type);
    }

    throw "Error: no such field";
  }

  virtual bool IsStruct() override {
    return true;
  }

  std::string_view Format() override {
    return struct_name_;
  }

 private:
  std::string_view struct_name_;
  std::vector<Member> members_;
};

//////////////////////////////////////////////////////////////////////

}  // namespace types
