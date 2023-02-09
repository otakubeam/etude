#include <types/type.hpp>

namespace types {

static Type::Arena type_store{};

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
      fmt::print(stderr, "Comparing unions!");
      std::abort();

    case TypeTag::TY_VARIABLE:
      fmt::print(stderr, "Comparing variables!");
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

void CheckTypes() {
  auto& store = type_store;
  for (auto& t : store) {
    if (t.tag == TypeTag::TY_APP) {
      if (!t.typing_context_) {
        fmt::print(stderr, "id:{} \t context:{:<20} \t\t\t type:{} \n", t.id,
                   (void*)t.typing_context_, FormatType(t));
        std::abort();
      } else if (!t.typing_context_->RetrieveSymbol(t.as_tyapp.name)) {
        fmt::print(stderr, "Could not find type locally\n");
        t.typing_context_->Print();
        fmt::print(stderr, "id:{} \t context:{:<20} \t\t\t type:{} \n", t.id,
                   (void*)t.typing_context_, FormatType(t));
        std::abort();
      }
    }
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
  type_store.push_back(Type{.id = type_store.size()});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTyCons(lex::Token name, std::vector<lex::Token> params, Type* body,
                 Type* kind, ast::scope::Context* context) {
  type_store.push_back(Type{.id = type_store.size(),
                                  .tag = TypeTag::TY_CONS,
                                  .typing_context_ = context,
                                  .as_tycons = {
                                      .name = name,
                                      .param_pack = std::move(params),
                                      .body = body,
                                      .kind = kind,
                                  }});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypeVar(ast::scope::Context* ty_cons) {
  type_store.push_back(Type{
      .id = type_store.size(),
      .typing_context_ = ty_cons,
  });

  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypePtr(Type* underlying) {
  type_store.push_back(Type{.id = type_store.size(),
                                  .tag = types::TypeTag::TY_PTR,
                                  .as_ptr = {.underlying = underlying}});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeFunType(std::vector<Type*> param_pack, Type* result_type) {
  type_store.push_back(
      Type{.id = type_store.size(),
           .tag = TypeTag::TY_FUN,
           .as_fun = {.param_pack = std::move(param_pack),
                      .result_type = result_type}});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTyApp(lex::Token name, std::vector<Type*> param_pack) {
  type_store.push_back(Type{
      .id = type_store.size(),
      .tag = TypeTag::TY_APP,
      .as_tyapp = {.name = name, .param_pack = std::move(param_pack)},
  });
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeStructType(std::vector<Member> fields) {
  type_store.push_back(Type{
      .tag = types::TypeTag::TY_STRUCT,
      .as_struct = {std::move(fields)},
  });
  return &type_store.back();
};

//////////////////////////////////////////////////////////////////////

Type* MakeSumType(std::vector<Member> fields) {
  type_store.push_back(Type{
      .tag = types::TypeTag::TY_SUM,
      .as_sum = {std::move(fields)},
  });
  return &type_store.back();
};

//////////////////////////////////////////////////////////////////////

void SetTyContext(types::Type* ty, ast::scope::Context* typing_context) {
  FMT_ASSERT(typing_context, "Not null");
  ty->typing_context_ = typing_context;

  switch (ty->tag) {
    case TypeTag::TY_PTR:
      SetTyContext(ty->as_ptr.underlying, typing_context);
      break;

    case TypeTag::TY_STRUCT:
      for (auto& member : ty->as_struct.first) {
        SetTyContext(member.ty, typing_context);
      }
      break;

    case TypeTag::TY_SUM:
      for (auto& member : ty->as_sum.first) {
        if (member.ty) {
          SetTyContext(member.ty, typing_context);
        }
      }
      break;

    case TypeTag::TY_FUN: {
      auto& pack = ty->as_fun.param_pack;
      for (auto& p : pack) {
        SetTyContext(p, typing_context);
      }
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

Type* FindLeader(Type* a) {
  if (a->leader) {
    return a->leader = FindLeader(a->leader);
  } else {
    return a;
  }
}

//////////////////////////////////////////////////////////////////////

using Map = std::unordered_map<std::string_view, Type*>;

Type* SubstituteParameters(Type* subs, const Map& map) {
  switch (subs->tag) {
    case TypeTag::TY_PTR: {
      auto underlying = SubstituteParameters(subs->as_ptr.underlying, map);

      auto ptr = MakeTypePtr(underlying);
      ptr->typing_context_ = subs->typing_context_;

      return ptr;
    }

    case TypeTag::TY_STRUCT: {
      std::vector<Member> result;
      auto& pack = subs->as_struct.first;

      for (auto& p : pack) {
        result.push_back(
            Member{.field = p.field, .ty = SubstituteParameters(p.ty, map)});
      }

      auto ty = MakeStructType(std::move(result));
      ty->typing_context_ = subs->typing_context_;

      return ty;
    }

    case TypeTag::TY_SUM: {
      std::vector<Member> result;
      auto& pack = subs->as_sum.first;

      for (auto& p : pack) {
        result.push_back(Member{.field = p.field,
                                .ty = p.ty  //
                                          ? SubstituteParameters(p.ty, map)
                                          : nullptr});
      }

      auto ty = MakeSumType(std::move(result));
      ty->typing_context_ = subs->typing_context_;

      return ty;
    }

    case TypeTag::TY_FUN: {
      std::vector<Type*> args;
      auto& pack = subs->as_fun.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        args.push_back(SubstituteParameters(pack[i], map));
      }

      Type* result = SubstituteParameters(subs->as_fun.result_type, map);

      auto ty = MakeFunType(std::move(args), result);
      ty->typing_context_ = subs->typing_context_;

      return ty;
    }

    case TypeTag::TY_APP: {
      // item: T             <<--- substitute

      if (map.contains(subs->as_tyapp.name)) {
        return map.at(subs->as_tyapp.name);
      }

      // next: List(T)       <<--- go inside

      std::vector<Type*> result;
      auto& pack = subs->as_tyapp.param_pack;

      for (auto& p : pack) {
        result.push_back(SubstituteParameters(p, map));
      }

      auto ty = MakeTyApp(subs->as_tyapp.name, std::move(result));
      ty->typing_context_ = subs->typing_context_;

      return ty;
    }

    case TypeTag::TY_CONS:
    case TypeTag::TY_UNION:
      std::abort();

    case TypeTag::TY_INT:
    case TypeTag::TY_BOOL:
    case TypeTag::TY_CHAR:
    case TypeTag::TY_UNIT:
    case TypeTag::TY_BUILTIN:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_KIND:
    default:
      return subs;
  }
}

//////////////////////////////////////////////////////////////////////

Type* ApplyTyconsLazy(Type* ty) {
  if (ty->tag != TypeTag::TY_APP) {
    return nullptr;
  }

  auto symbol = ty->typing_context_->RetrieveSymbol(ty->as_tyapp.name);
  auto& names = symbol->GetType()->as_tycons.param_pack;

  auto& pack = ty->as_tyapp.param_pack;

  if (pack.size() != names.size()) {
    throw std::runtime_error("Instantination size mismatch");
  }

  std::unordered_map<std::string_view, Type*> map;
  for (size_t i = 0; i < pack.size(); i++) {
    map.insert({names[i], pack[i]});
  }

  auto subs = SubstituteParameters(symbol->GetType()->as_tycons.body, map);

  return subs;
}

//////////////////////////////////////////////////////////////////////

Type* TypeStorage(Type* t) {
  while (t->tag == TypeTag::TY_APP) {
    t = ApplyTyconsLazy(t);
  }

  return t;
}

//////////////////////////////////////////////////////////////////////

// Ty here is a type schema
Type* Instantinate(Type* ty, KnownParams& map) {
  auto l = FindLeader(ty);

  switch (l->tag) {
    case TypeTag::TY_VARIABLE:
      return l;  // TODO: idk, when should I instantiate in recursive defs?

    case TypeTag::TY_PTR: {
      auto i = Instantinate(l->as_ptr.underlying, map);
      auto ptr = MakeTypePtr(i);
      ptr->typing_context_ = l->typing_context_;
      return ptr;
    }

    case TypeTag::TY_PARAMETER:
      if (map.contains(l)) {
        return map.at(l);
      }
      return map[l] = MakeTypeVar();

    case TypeTag::TY_APP: {
      std::vector<Type*> args;

      auto& pack = l->as_tyapp.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        args.push_back(Instantinate(pack[i], map));
      }

      auto app = MakeTyApp(l->as_tyapp.name, std::move(args));
      app->typing_context_ = l->typing_context_;

      return app;
    }

    case TypeTag::TY_FUN: {
      std::vector<Type*> args;

      auto& pack = l->as_fun.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        args.push_back(Instantinate(pack[i], map));
      }

      auto fun = MakeFunType(std::move(args),
                             Instantinate(l->as_fun.result_type, map));
      fun->typing_context_ = l->typing_context_;

      return fun;
    }

    case TypeTag::TY_STRUCT: {
      std::vector<Member> args;
      auto& pack = l->as_struct.first;

      for (auto& p : pack) {
        args.push_back(Member{
            .field = p.field,
            .ty = Instantinate(p.ty, map),
        });
      }

      auto ty = MakeStructType(std::move(args));
      ty->typing_context_ = l->typing_context_;
      return ty;
    }

    case TypeTag::TY_SUM: {
      std::vector<Member> args;
      auto& pack = l->as_sum.first;

      for (auto& p : pack) {
        args.push_back(Member{
            .field = p.field,
            .ty = Instantinate(p.ty, map),
        });
      }

      auto ty = MakeSumType(std::move(args));
      ty->typing_context_ = l->typing_context_;
      return ty;
    }

    case TypeTag::TY_CONS:
    case TypeTag::TY_UNION:
      std::abort();
      break;

    case TypeTag::TY_KIND:
    default:
      return l;  // Int, Bool, Unit, etc
  }
}

}  // namespace types
