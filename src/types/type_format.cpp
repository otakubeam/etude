#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

std::string FormatStruct(StructTy* type) {
  std::string result;
  auto insert = std::back_inserter(result);

  fmt::format_to(insert, "struct {{ ");

  for (auto a = type->members; a; a = a->next) {
    fmt::format_to(insert, "{}: {}, ",  //
                   a->field,            //
                   FormatType(a->ty));
  }

  fmt::format_to(insert, "}}");

  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatSum(StructTy* type) {
  std::string result;
  auto insert = std::back_inserter(result);

  fmt::format_to(insert, "sum {{ ");

  for (auto a = type->members; a; a = a->next) {
    fmt::format_to(insert,
                   "{}: {} | ",               //
                   a->field,                  //
                   a->ty ? FormatType(a->ty)  //
                         : "()");
  }

  fmt::format_to(insert, "}}");

  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatApp(TyAppType* type) {
  if (!type->parameters) {
    return std::string{type->name};  // Nullary type application
  }

  std::string result;
  auto a = type->parameters;
  auto insert = std::back_inserter(result);

  fmt::format_to(insert, "{}(", type->name);

  for (auto a = type->parameters; a->next; a = a->next) {
    fmt::format_to(insert, "{}, ", FormatType(a->ty));
  }

  fmt::format_to(insert, "{})", FormatType(a->ty));

  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatFun(FunType* type) {
  std::string result;
  auto insert = std::back_inserter(result);

  fmt::format_to(insert, "->");

  for (auto a = type->parameters; a; a = a->next) {
    fmt::format_to(insert, "{} -> ", FormatType(a->ty));
  }

  fmt::format_to(insert, "{}", FormatType(type->result_type));

  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatPtr(PtrType* type) {
  return fmt::format("*{}", FormatType(type->underlying));
}

//////////////////////////////////////////////////////////////////////

std::string FormatCons(TyConsType* type) {
  return fmt::format("Cons {}", type->name);
}

//////////////////////////////////////////////////////////////////////

std::string FormatTypeInner(Type* type) {
  switch (type->tag) {
    case TypeTag::TY_KIND:
      return fmt::format("*");
    case TypeTag::TY_NEVER:
      return fmt::format("!");
    case TypeTag::TY_INT:
      return fmt::format("Int");
    case TypeTag::TY_BOOL:
      return fmt::format("Bool");
    case TypeTag::TY_CHAR:
      return fmt::format("Char");
    case TypeTag::TY_UNIT:
      return fmt::format("Unit");

    case TypeTag::TY_PTR:
      return FormatPtr(&type->as_ptr);
    case TypeTag::TY_FUN:
      return FormatFun(&type->as_fun);
    case TypeTag::TY_APP:
      return FormatApp(&type->as_tyapp);
    case TypeTag::TY_SUM:
      return FormatSum(&type->as_struct);
    case TypeTag::TY_CONS:
      return FormatCons(&type->as_tycons);
    case TypeTag::TY_STRUCT:
      return FormatStruct(&type->as_struct);

    case TypeTag::TY_VARIABLE:
      return fmt::format("${}{}",  //
                         FormatConstraints(type->as_var.constraints),
                         type->as_var.id);

    case TypeTag::TY_PARAMETER:
      return fmt::format("G{}{}",  //
                         FormatConstraints(type->as_var.constraints),
                         type->as_var.id);

    case TypeTag::TY_UNION:
    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

std::string FormatConstraints(Trait* constraints) {
  std::string output;

  auto insert = std::back_inserter(output);

  for (; constraints; constraints = constraints->next) {
    fmt::format_to(insert, "{}::", FormatTraitNoType(*constraints));
  }

  return output;
}

//////////////////////////////////////////////////////////////////////

std::string FormatType(Type* type) {
  return FormatTypeInner(FindLeader(type));
}

//////////////////////////////////////////////////////////////////////

std::string FormatTypeDebug(Type* type) {
  if (FindLeader(type) == type) {
    return fmt::format("{}", FormatType(type));
  }

  return fmt::format("({} => {})",      //
                     FormatType(type),  //
                     FormatType(FindLeader(type)));
}

//////////////////////////////////////////////////////////////////////

};  // namespace types
