#pragma once

#include <types/type.hpp>

namespace qbe::detail {

class SizeMeasure {
 public:
  bool IsStruct(types::Type* t) {
    while (t->tag == types::TypeTag::TY_APP) {
      t = types::ApplyTyconsLazy(t);
    }

    return t->tag == types::TypeTag::TY_STRUCT;
  }

  size_t MeasureAlignment(types::Type* t) {
    t = types::FindLeader(t);

    switch (t->tag) {
      case types::TypeTag::TY_INT:
        return 4;

      case types::TypeTag::TY_BOOL:
      case types::TypeTag::TY_CHAR:
        return 1;

      case types::TypeTag::TY_UNIT:
      case types::TypeTag::TY_PTR:
      case types::TypeTag::TY_FUN:
        return 8;

      case types::TypeTag::TY_APP: {
        while (t->tag == types::TypeTag::TY_APP) {
          t = types::ApplyTyconsLazy(t);
        }
        return MeasureAlignment(t);
      }

      case types::TypeTag::TY_STRUCT: {
        size_t result = 4;
        for (auto& f : t->as_struct.first) {
          auto align = MeasureAlignment(f.ty);
          result = result > align ? result : align;
        }
        return result;
      }

      default:
        FMT_ASSERT(false, "Unreachable!");
    }
  }

  size_t MeasureFieldOffset(types::Type* t, std::string_view field) {
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
        return 4;

      case types::TypeTag::TY_BOOL:
      case types::TypeTag::TY_CHAR:
        return 1;

      case types::TypeTag::TY_UNIT:
      case types::TypeTag::TY_PTR:
      case types::TypeTag::TY_FUN:
        return 8;

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

}  // namespace qbe::detail
