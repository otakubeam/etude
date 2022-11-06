#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

struct NotEquivalentError : std::exception {
  std::string message = "Types are not equivalent";

  const char* what() const noexcept override {
    return message.c_str();
  }
};

//////////////////////////////////////////////////////////////////////

bool EqApp(TyAppType* lhs, TyAppType* rhs) {
  auto& pack_a = lhs->param_pack;
  auto& pack_b = rhs->param_pack;

  if (lhs->name.GetName() != rhs->name.GetName()) {
    return false;
  }

  if (pack_a.size() != pack_b.size()) {
    return false;
  }

  for (size_t i = 0; i < pack_a.size(); i++) {
    if (!TypesEquivalent(pack_a[i], pack_b[i])) {
      return false;
    }
  }

  return true;
}

bool EqFun(FunType* lhs, FunType* rhs) {
  auto& pack_a = lhs->param_pack;
  auto& pack_b = rhs->param_pack;

  if (pack_a.size() != pack_b.size()) {
    return false;
  }

  for (size_t i = 0; i < pack_a.size(); i++) {
    if (!TypesEquivalent(pack_a[i], pack_b[i])) {
      return false;
    }
  }

  return TypesEquivalent(lhs->result_type, rhs->result_type);
}

bool EqStr(StructTy* lhs, StructTy* rhs) {
  auto& pack_a = lhs->first;
  auto& pack_b = rhs->first;

  if (pack_a.size() != pack_b.size()) {
    return false;
  }

  for (size_t i = 0; i < pack_a.size(); i++) {
    if (pack_a[i].field != pack_b[i].field) {
      return false;
    }

    if (!TypesEquivalent(pack_a[i].ty, pack_b[i].ty)) {
      return false;
    }
  }

  return true;
}

bool TypesEquivalent(Type* lhs, Type* rhs,
                     std::unordered_map<size_t, size_t> map) {
  lhs = FindLeader(lhs);
  rhs = FindLeader(rhs);

  if (lhs->tag != rhs->tag) {
    return false;
  }

  switch (lhs->tag) {
    case TypeTag::TY_APP:
      return EqApp(&lhs->as_tyapp, &rhs->as_tyapp);

    case TypeTag::TY_FUN:
      return EqFun(&lhs->as_fun, &rhs->as_fun);

    case TypeTag::TY_PTR:
      return TypesEquivalent(lhs->as_ptr.underlying, rhs->as_ptr.underlying,
                             map);

    case TypeTag::TY_STRUCT:
      return EqStr(&lhs->as_struct, &rhs->as_struct);

    case TypeTag::TY_UNION:
      fmt::print("Comparing unions!");
      std::abort();

    case TypeTag::TY_VARIABLE:
      fmt::print("Comparing variables!");
      return false;

    case TypeTag::TY_PARAMETER:
      if (map.contains(rhs->id)) {
        if (lhs->id != map.at(rhs->id)) {
          return false;
        }
      } else {
        map[rhs->id] = lhs->id;
      }
      return true;

    case TypeTag::TY_INT:
    case TypeTag::TY_BOOL:
    case TypeTag::TY_CHAR:
    case TypeTag::TY_UNIT:
    case TypeTag::TY_BUILTIN:
    case TypeTag::TY_KIND:
      return true;

    case TypeTag::TY_CONS:
    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

void PrintTypeStore() {
  auto& store = Type::type_store;
  fmt::print("[!] Type store\n\n");
  for (auto& t : store) {
    fmt::print("id:{} \t context:{:<20} \t\t\t type:{} \n", t.id,
               (void*)t.typing_context_, FormatType(t));
  }
}

//////////////////////////////////////////////////////////////////////

Type* HintedOrNew(Type* type) {
  return type ? type : MakeTypeVar();
}

//////////////////////////////////////////////////////////////////////

auto MakeKindParamPack(size_t size) -> std::vector<Type*> {
  std::vector<Type*> result;
  for (size_t i = 0; i < size; i++) {
    result.push_back(&builtin_kind);
  }
  return result;
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypeVar() {
  Type::type_store.push_back(Type{.id = Type::type_store.size()});
  return &Type::type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTyCons(lex::Token name, std::vector<lex::Token> params, Type* body,
                 Type* kind, ast::scope::Context* context) {
  Type::type_store.push_back(Type{.id = Type::type_store.size(),
                                  .tag = TypeTag::TY_CONS,
                                  .typing_context_ = context,
                                  .as_tycons = {
                                      .name = name,
                                      .param_pack = std::move(params),
                                      .body = body,
                                      .kind = kind,
                                  }});
  return &Type::type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypeVar(ast::scope::Context* ty_cons) {
  Type::type_store.push_back(Type{
      .id = Type::type_store.size(),
      .typing_context_ = ty_cons,
  });

  return &Type::type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypePtr(Type* underlying) {
  Type::type_store.push_back(Type{.id = Type::type_store.size(),
                                  .tag = types::TypeTag::TY_PTR,
                                  .as_ptr = {.underlying = underlying}});
  return &Type::type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeFunType(std::vector<Type*> param_pack, Type* result_type) {
  Type::type_store.push_back(
      Type{.id = Type::type_store.size(),
           .tag = TypeTag::TY_FUN,
           .as_fun = {.param_pack = std::move(param_pack),
                      .result_type = result_type}});
  return &Type::type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTyApp(lex::Token name, std::vector<Type*> param_pack) {
  Type::type_store.push_back(Type{
      .id = Type::type_store.size(),
      .tag = TypeTag::TY_APP,
      .as_tyapp = {.name = name, .param_pack = std::move(param_pack)},
  });
  return &Type::type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeStructType(std::vector<Member> fields) {
  Type::type_store.push_back(Type{
      .tag = types::TypeTag::TY_STRUCT,
      .as_struct = {std::move(fields)},
  });
  return &Type::type_store.back();
};

//////////////////////////////////////////////////////////////////////

void SetTyContext(types::Type* ty, ast::scope::Context* typing_context) {
  ty->typing_context_ = typing_context;

  switch (ty->tag) {
    case TypeTag::TY_PTR:
      SetTyContext(ty->as_ptr.underlying, typing_context);
      break;

    case TypeTag::TY_STRUCT:
      for (auto& member : ty->as_struct.first)
        SetTyContext(member.ty, typing_context);
      break;

    case TypeTag::TY_FUN: {
      auto& pack = ty->as_fun.param_pack;
      for (auto& p : pack) SetTyContext(p, typing_context);
      SetTyContext(ty->as_fun.result_type, typing_context);
      break;
    }

    case TypeTag::TY_APP:
      for (auto& arg : ty->as_tyapp.param_pack)
        SetTyContext(arg, typing_context);
      break;

    case TypeTag::TY_UNION:
      std::abort();

    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_CONS:
    case TypeTag::TY_KIND:
    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

}  // namespace types
// namespace types
