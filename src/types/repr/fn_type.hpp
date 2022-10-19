#pragma once

#include <types/repr/builtins.hpp>

#include <types/type.hpp>

#include <fmt/format.h>

namespace types {

class FnType : public Type {
 public:
  FnType() = default;

  FnType(std::vector<Type*> arg_types, Type* return_type)
      : arg_types_{arg_types}, return_type_{return_type} {
    FMT_ASSERT(return_type_, "Nullptr return type");
  }

  bool IsEqual(Type* other) override {
    return other->IsEqual(this);
  }

  bool IsEqual(PointerType*) override {
    return false;
  }

  bool IsEqual(BuiltinType*) override {
    // TODO: is builtin a function taking no arguments?
    return false;
  }

  bool IsEqual(StructType*) override {
    return false;
  }

  bool IsEqual(FnType* other) override {
    if (!return_type_->IsEqual(other->return_type_) ||
        arg_types_.size() != other->arg_types_.size()) {
      return false;
    }

    for (std::size_t i = 0; i < arg_types_.size(); i++) {
      if (!arg_types_[i]->IsEqual(other->arg_types_[i])) {
        return false;
      }
    }

    return true;
  }

  Type* GetReturnType() {
    return return_type_;
  }

  std::string_view Format() override {
    if (format_view_.empty()) {
      InitFormatView();
    }
    return format_view_;
  }

 private:
  void InitFormatView() {
    for (auto arg : arg_types_) {
      auto inserter = std::back_inserter(format_view_);
      fmt::format_to(inserter, "{} -> ", arg->Format());
    }
    auto inserter = std::back_inserter(format_view_);
    fmt::format_to(inserter, "{}", return_type_->Format());
  }

 private:
  std::vector<Type*> arg_types_;

  Type* return_type_ = &builtin_unit;

  std::string format_view_{};
};

}  // namespace types
