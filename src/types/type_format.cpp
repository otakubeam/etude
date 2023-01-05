#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

std::string FormatStruct(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "struct {{ ");

  for (auto& a : type.as_struct.first) {
    fmt::format_to(insert, "{}: {}, ", a.field, FormatType(*a.ty));
  }

  fmt::format_to(insert, "}}");
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatSum(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "sum {{ ");

  for (auto& a : type.as_sum.first) {
    fmt::format_to(insert, "{}: {} | ", a.field,
                   a.ty ? FormatType(*a.ty) : "()");
  }

  fmt::format_to(insert, "}}");
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatApp(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);

  auto& tyapp = type.as_tyapp;

  if (tyapp.param_pack.empty()) {
    fmt::format_to(insert, "{}", tyapp.name.GetName());
    return result;
  }

  fmt::format_to(insert, "{}( ", tyapp.name.GetName());

  for (auto& a : tyapp.param_pack) {
    fmt::format_to(insert, "{}, ", FormatType(*a));
  }

  fmt::format_to(insert, ")");
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatFun(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "(");

  for (auto& a : type.as_fun.param_pack) {
    fmt::format_to(insert, "{} -> ", FormatType(*a));
  }

  fmt::format_to(insert, "{})", FormatType(*type.as_fun.result_type));
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatPtr(Type& type) {
  return fmt::format("*{}", FormatType(*type.as_ptr.underlying));
}

//////////////////////////////////////////////////////////////////////

std::string FormatUnion(Type&) {
  return fmt::format("union {{ <unimplemented> }}");
}

//////////////////////////////////////////////////////////////////////

std::string FormatCons(Type& type) {
  return fmt::format(
      "Cons {} of {} of {}", type.as_tycons.name.GetName(),
      type.as_tycons.kind ? FormatType(*type.as_tycons.kind) : "<kind>",
      FormatType(*type.as_tycons.body));
}

//////////////////////////////////////////////////////////////////////

std::string FormatTypeInner(Type& type) {
  switch (type.tag) {
    case TypeTag::TY_KIND:
      return fmt::format("*");
    case TypeTag::TY_INT:
      return fmt::format("Int");
    case TypeTag::TY_BOOL:
      return fmt::format("Bool");
    case TypeTag::TY_CHAR:
      return fmt::format("Char");
    case TypeTag::TY_UNIT:
      return fmt::format("Unit");
    case TypeTag::TY_NEVER:
      return "!";

    case TypeTag::TY_PTR:
      return FormatPtr(type);
    case TypeTag::TY_UNION:
      return FormatUnion(type);
    case TypeTag::TY_SUM:
      return FormatSum(type);
    case TypeTag::TY_STRUCT:
      return FormatStruct(type);
    case TypeTag::TY_FUN:
      return FormatFun(type);
    case TypeTag::TY_CONS:
      return FormatCons(type);
    case TypeTag::TY_APP:
      return FormatApp(type);

    case TypeTag::TY_VARIABLE:
      return fmt::format("${}", type.id);

    case TypeTag::TY_PARAMETER:
      return fmt::format("G{}", type.id);

    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

std::string MangleFun(Type& type) {
  std::string result;
  auto ins = std::back_inserter(result);
  fmt::format_to(ins, "f{}", type.as_fun.param_pack.size());
  for (auto& t : type.as_fun.param_pack) {
    fmt::format_to(ins, "{}", Mangle(*t));
  }

  return result;
}

std::string MangleApp(Type& type) {
  std::string result;
  auto ins = std::back_inserter(result);
  fmt::format_to(ins, "{}", type.as_tyapp.name.GetName());
  for (auto& t : type.as_tyapp.param_pack) {
    fmt::format_to(ins, "{}", Mangle(*t));
  }
  return result;
}

std::string MangleStruct(Type& type) {
  std::string result;
  auto ins = std::back_inserter(result);
  fmt::format_to(ins, "struct");
  for (auto& t : type.as_struct.first) {
    fmt::format_to(ins, "{}", Mangle(*t.ty));
  }
  return result;
}

std::string Mangle(Type& type) {
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

std::string FormatConstraints(Type& type) {
  std::string output;
  auto ins = std::back_inserter(output);
  for (auto& cons : type.as_parameter.constraints) {
    fmt::format_to(ins, "{}::", (int)cons.tag);
  }
  return output;
}

std::string FormatType(Type& type) {
  if (auto clean = true) {
    auto& leader = *FindLeader(&type);
    return fmt::format("{}{}", FormatConstraints(leader),
                       FormatTypeInner(leader));
  }

  if (FindLeader(&type) == &type) {
    return fmt::format("{}{}", FormatConstraints(type), FormatTypeInner(type));
  }

  return fmt::format("({}{} => {}{})", FormatConstraints(type),
                     FormatTypeInner(type),
                     FormatConstraints(*FindLeader(&type)),
                     FormatTypeInner(*FindLeader(&type)));
}

//////////////////////////////////////////////////////////////////////

};  // namespace types
