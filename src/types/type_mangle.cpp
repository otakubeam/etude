#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

std::string MangleFun(FunType* type) {
  std::string result;
  auto ins = std::back_inserter(result);

  fmt::format_to(ins, "fn");

  for (auto& t : type.as_fun.param_pack) {
    fmt::format_to(ins, "{}", Mangle(*t));
  }

  return result;
}

//////////////////////////////////////////////////////////////////////

std::string MangleApp(Type* type) {
  std::string result;
  auto ins = std::back_inserter(result);
  fmt::format_to(ins, "{}", type.as_tyapp.name);
  for (auto& t : type.as_tyapp.param_pack) {
    fmt::format_to(ins, "{}", Mangle(*t));
  }
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string MangleStruct(Type* type) {
  std::string result;
  auto ins = std::back_inserter(result);
  fmt::format_to(ins, "struct");
  for (auto& t : type.as_struct.first) {
    fmt::format_to(ins, "{}", Mangle(*t.ty));
  }
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string Mangle(Type* type) {
  switch (type.tag) {
    case TypeTag::TY_INT:
      return fmt::format("i");
    case TypeTag::TY_BOOL:
      return fmt::format("b");
    case TypeTag::TY_CHAR:
      return fmt::format("c");
    case TypeTag::TY_UNIT:
      return fmt::format("u");

    case TypeTag::TY_PTR:
      return "P" + Mangle(*type.as_ptr.underlying);
    case TypeTag::TY_FUN:
      return MangleFun(type);
    case TypeTag::TY_APP:
      return MangleApp(type);

    case TypeTag::TY_STRUCT:
      return MangleStruct(type);

    case TypeTag::TY_SUM:
    case TypeTag::TY_UNION:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_CONS:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_KIND:
    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

};  // namespace types
