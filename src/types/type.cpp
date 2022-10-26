#include <types/type.hpp>

namespace types {

std::string FormatType(Type& type);

std::string FormatStruct(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "struct {{ ");
  for (auto a : type.as_struct.first) {
    fmt::format_to(insert, "{}: {}, ", a.field, FormatType(*a.ty));
  }
  fmt::format_to(insert, "}};");
  return result;
}

std::string FormatFun(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "(");
  for (auto a : type.as_fun.param_pack) {
    fmt::format_to(insert, " {} -> ", FormatType(*a));
  }
  fmt::format_to(insert, "{} )", FormatType(*type.as_fun.result_type));
  return result;
}

Type* FindLeader(Type* a) {
  auto leader = a;
  while (leader->leader) {
    leader = leader->leader;
  }
  return leader;
}

std::string FormatType(Type& type) {
  if (type.leader) {
    return FormatType(*FindLeader(&type));
  }

  switch (type.tag) {
    case TypeTag::TY_INT:
      return fmt::format("Int");
    case TypeTag::TY_BOOL:
      return fmt::format("Bool");
    case TypeTag::TY_CHAR:
      return fmt::format("Char");
    case TypeTag::TY_UNIT:
      return fmt::format("Unit");

    case TypeTag::TY_PTR:
      return fmt::format("*{}", FormatType(*type.as_ptr.underlying));
    case TypeTag::TY_UNION:
      return fmt::format("union {{ idk }}");
    case TypeTag::TY_STRUCT:
      return FormatStruct(type);

    case TypeTag::TY_FUN:
      return FormatFun(type);

    case TypeTag::TY_ALIAS:
      return std::string(type.as_alias.name) +
             (type.as_alias.underlying ? FormatType(*type.as_alias.underlying)
                                       : "");

    case TypeTag::TY_VARIABLE:
      return fmt::format("Var:{}", (void*)&type);

    case TypeTag::TY_PARAMETER:
    default:
      std::abort();
  }
}

Type builtin_int{.tag = TypeTag::TY_INT};
Type builtin_bool{.tag = TypeTag::TY_BOOL};
Type builtin_char{.tag = TypeTag::TY_CHAR};
Type builtin_unit{.tag = TypeTag::TY_UNIT};

void Unify(Type* a, Type* b);

void UnifyUnderlyingTypes(Type* a, Type* b) {
  // assert(la->tag == lb->tag);
  switch (a->tag) {
    case TypeTag::TY_PTR:
      fmt::print("PTR\n");
      Unify(a->as_ptr.underlying, b->as_ptr.underlying);
      break;

    case TypeTag::TY_ALIAS:
      fmt::print("Alias\n");
      if (a->as_alias.name != b->as_alias.name) {
        throw;
      }
      Unify(a->as_alias.underlying, b->as_alias.underlying);
      break;

    case TypeTag::TY_STRUCT: {
      if (a->as_struct.first.size() != b->as_struct.first.size()) {
        throw "";
      }

      for (size_t i = 0; i < a->as_struct.first.size(); i++) {
        // TODO:
      }

      std::abort();
      break;
    }

    case TypeTag::TY_FUN:
      // Unify(a->as_fun.param_pack, b->as_fun.param_pack);
      // Unify(a->as_fun.result_type, b->as_fun.result_type);

      std::abort();
      break;

    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_UNION:
      std::abort();

    default:
      fmt::print("BREAK\n");
      break;
  }
}

void Unify(Type* a, Type* b) {
  fmt::print("Unifying {} and {}\n", FormatType(*a), FormatType(*b));

  auto la = FindLeader(a);
  auto lb = FindLeader(b);

  fmt::print("Their leaders: {} and {}\n", FormatType(*la), FormatType(*lb));
  fmt::print("Here\n");

  // Always make the la be be a variable
  if (lb->tag == TypeTag::TY_VARIABLE) {
    std::swap(la, lb);
  }
  fmt::print("Here\n");

  if (la->tag == TypeTag::TY_VARIABLE) {
    la->leader = lb;
    fmt::print("Set leader");
    // la->as_variable.constraints + lb->as_variable.constraints
    return;
  }
  fmt::print("Here\n");

  if (la->tag == lb->tag) {
    fmt::print("Compared tags\n");

    UnifyUnderlyingTypes(la, lb);

    fmt::print("Unified\n");
    return;
  }

  throw "error";  // Error!
}

//////////////////////////////////////////////////////////////////////
};  // namespace types
