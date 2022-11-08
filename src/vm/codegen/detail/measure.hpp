#pragma once

#include <types/type.hpp>

namespace vm::codegen::detail {

class SizeMeasure {
 public:
  size_t MeasureFieldOffset(types::Type* t, std::string_view field) {
    fmt::print("Measuring {} for {}\n", types::FormatType(*t), field);

    while (t->tag == types::TypeTag::TY_APP) {
      t = types::ApplyTyconsLazy(t);
    }

    FMT_ASSERT(t->tag == types::TypeTag::TY_STRUCT,
               "Typeoffset is not a strucct");  // This should be unreachable

    size_t offset = 0;

    for (auto& mem : t->as_struct.first) {
      if (mem.field.compare(field) == 0) {
        return offset;
      } else {
        offset += MeasureSize(mem.ty);
      }
    }

    throw "Could not find field";
  }

  size_t MeasureSize(types::Type* t) {
    t = types::FindLeader(t);

    switch (t->tag) {
      case types::TypeTag::TY_INT:
      case types::TypeTag::TY_BOOL:
      case types::TypeTag::TY_CHAR:
      case types::TypeTag::TY_UNIT:
      case types::TypeTag::TY_PTR:
      case types::TypeTag::TY_FUN:
        return 1;

      case types::TypeTag::TY_APP:
        return MeasureTyApp(t);

      case types::TypeTag::TY_STRUCT:
        return MeasureStruct(t);

      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  size_t MeasureTyApp(types::Type* t) {
    FMT_ASSERT(t->tag == types::TypeTag::TY_APP, "Incorrect tag");
    return MeasureSize(types::ApplyTyconsLazy(t));
  }

  size_t MeasureStruct(types::Type* t) {
    FMT_ASSERT(t->tag == types::TypeTag::TY_STRUCT, "Incorrect tag");
    auto result = 0;
    for (auto& mem : t->as_struct.first) {
      result += MeasureSize(mem.ty);
    }
    return result;
  }

 private:
  // cache, etc..
};

}  // namespace vm::codegen::detail
