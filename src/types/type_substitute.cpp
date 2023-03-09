#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////
//                      Substitutions
//////////////////////////////////////////////////////////////////////

Type* SubstPtrType(PtrType* type, const Map& map) {
  auto underlying = SubstituteParameters(type->underlying, map);
  return MakeTypePtr(underlying);
}

//////////////////////////////////////////////////////////////////////

Member* SubstMembersList(Member* type, const Map& map) {
  return type ? new Member{
                  .ty = SubstituteParameters(type->ty, map),
                  .field = type->field,
                  .next = SubstMembersList(type->next, map),
              }
              : nullptr;
}

//////////////////////////////////////////////////////////////////////

Type* SubstStructType(StructTy* type, const Map& map) {
  return MakeStructType(SubstMembersList(type->members, map));
}

//////////////////////////////////////////////////////////////////////

Type* SubstSumType(StructTy* type, const Map& map) {
  return MakeSumType(SubstMembersList(type->members, map));
}

//////////////////////////////////////////////////////////////////////

Parameter* SubstParameterList(Parameter* type, const Map& map) {
  return type ? new Parameter{
                  .ty = SubstituteParameters(type->ty, map),
                  .next = SubstParameterList(type->next, map),
              }
              : nullptr;
}

//////////////////////////////////////////////////////////////////////

Type* SubstFunType(FunType* type, const Map& map) {
  return MakeFunType(SubstParameterList(type->parameters, map),
                     SubstituteParameters(type->result_type, map));
}

//////////////////////////////////////////////////////////////////////

Type* SubstAppType(TyAppType* type, const Map& map) {
  // item: T             <<--- substitute

  if (map.contains(type->name)) {
    return map.at(type->name);
  }

  // next: List(T)       <<--- go inside

  auto pack = SubstParameterList(type->parameters, map);

  return MakeTyApp(type->name, pack);
}

//////////////////////////////////////////////////////////////////////

Type* SubstituteParameters(Type* subs, const Map& map) {
  switch (subs->tag) {
    case TypeTag::TY_PTR:
      return SubstPtrType(&subs->as_ptr, map);

    case TypeTag::TY_SUM:
      return SubstSumType(&subs->as_struct, map);

    case TypeTag::TY_STRUCT:
      return SubstStructType(&subs->as_struct, map);

    case TypeTag::TY_FUN:
      return SubstFunType(&subs->as_fun, map);

    case TypeTag::TY_APP:
      return SubstAppType(&subs->as_tyapp, map);

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
//                          Freshening
//////////////////////////////////////////////////////////////////////

Type* InstPtrType(PtrType* type, const KnownParams& map) {
  auto underlying = InstituteParameters(type->underlying, map);
  return MakeTypePtr(underlying);
}

//////////////////////////////////////////////////////////////////////

Member* InstMembersList(Member* type, const KnownParams& map) {
  return type ? new Member{
                  .ty = InstituteParameters(type->ty, map),
                  .field = type->field,
                  .next = InstMembersList(type->next, map),
              }
              : nullptr;
}

//////////////////////////////////////////////////////////////////////

Type* InstStructType(StructTy* type, const KnownParams& map) {
  return MakeStructType(InstMembersList(type->members, map));
}

//////////////////////////////////////////////////////////////////////

Type* InstSumType(StructTy* type, const KnownParams& map) {
  return MakeSumType(InstMembersList(type->members, map));
}

//////////////////////////////////////////////////////////////////////

Parameter* InstParameterList(Parameter* type, const KnownParams& map) {
  return type ? new Parameter{
                  .ty = InstituteParameters(type->ty, map),
                  .next = InstParameterList(type->next, map),
              }
              : nullptr;
}

//////////////////////////////////////////////////////////////////////

Type* InstFunType(FunType* type, const KnownParams& map) {
  return MakeFunType(InstParameterList(type->parameters, map),
                     InstituteParameters(type->result_type, map));
}

//////////////////////////////////////////////////////////////////////

Type* InstAppType(TyAppType* type, const KnownParams& map) {
  auto pack = InstParameterList(type->parameters, map);
  return MakeTyApp(type->name, pack);
}

//////////////////////////////////////////////////////////////////////

// Ty here is a type schema
Type* InstituteParameters(Type* type, KnownParams& map) {
  auto subs = FindLeader(type);

  switch (subs->tag) {
    case TypeTag::TY_PTR:
      return InstPtrType(&subs->as_ptr, map);

    case TypeTag::TY_SUM:
      return InstSumType(&subs->as_struct, map);

    case TypeTag::TY_STRUCT:
      return InstStructType(&subs->as_struct, map);

    case TypeTag::TY_FUN:
      return InstFunType(&subs->as_fun, map);

    case TypeTag::TY_APP:
      return InstAppType(&subs->as_tyapp, map);

    case TypeTag::TY_PARAMETER:
      return map.contains(subs) ? map.at(subs)  //
                                : map[subs] = MakeTypeVar();

    case TypeTag::TY_CONS:
    case TypeTag::TY_UNION:
      std::abort();

    case TypeTag::TY_INT:
    case TypeTag::TY_BOOL:
    case TypeTag::TY_CHAR:
    case TypeTag::TY_UNIT:
    case TypeTag::TY_BUILTIN:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_KIND:
    default:
      return subs;
  }
}

//////////////////////////////////////////////////////////////////////
//                        Application
//////////////////////////////////////////////////////////////////////

Type* ApplyTyconsLazy(Type* ty) {
  if (ty->tag != TypeTag::TY_APP) {
    return nullptr;
  }

  auto arg = ty->as_tyapp.parameters;
  auto symbol = ty->typing_context_->RetrieveSymbol(ty->as_tyapp.name);
  auto& parameters = symbol->GetType()->as_tycons.param_pack;

  std::unordered_map<std::string_view, Type*> map;
  for (size_t i = 0; i < parameters.size(); arg = arg->next, i++) {
    map.insert({parameters[i], arg->ty});
  }

  auto subs = SubstituteParameters(symbol->GetType()->as_tycons.body, map);

  SetTyContext(subs, ty->typing_context_);

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

}  // namespace types
