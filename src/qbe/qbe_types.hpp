#pragma once

#include <types/type.hpp>

namespace qbe {

////////////////////////////////////////////////////////////////////

inline std::string ToQbeType(types::Type* type) {
  switch (type->tag) {
    case types::TypeTag::TY_INT:
    case types::TypeTag::TY_CHAR:
    case types::TypeTag::TY_UNIT:
    case types::TypeTag::TY_BOOL:
      return "w";

    case types::TypeTag::TY_PTR:
      return "l";

    case types::TypeTag::TY_APP:
      return ":" + types::Mangle(*type);

    default:
      std::abort();
  }
}

inline std::string CopySuf(types::Type* type) {
  switch (type->tag) {
    case types::TypeTag::TY_INT:
    case types::TypeTag::TY_CHAR:
    case types::TypeTag::TY_UNIT:
    case types::TypeTag::TY_BOOL:
      return "w";

    case types::TypeTag::TY_PTR:
    case types::TypeTag::TY_APP:
      return "l";

    default:
      std::abort();
  }
}


inline std::string_view LoadSuf(types::Type* ty) {
  switch (ty->tag) {
    case types::TypeTag::TY_INT:
      return "sw";

    case types::TypeTag::TY_CHAR:
    case types::TypeTag::TY_UNIT:
    case types::TypeTag::TY_BOOL:
      return "ub";

    case types::TypeTag::TY_PTR:
      return "l";

    default:
      std::abort();
  }
}

inline std::string_view StoreSuf(types::Type* ty) {
  switch (ty->tag) {
    case types::TypeTag::TY_INT:
      return "w";

    case types::TypeTag::TY_CHAR:
    case types::TypeTag::TY_UNIT:
    case types::TypeTag::TY_BOOL:
      return "b";

    case types::TypeTag::TY_PTR:
      return "l";

    default:
      std::abort();
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace qbe
