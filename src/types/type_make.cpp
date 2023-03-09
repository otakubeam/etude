#include <types/type.hpp>

namespace types {

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
  return MakeTypeVar(nullptr);
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypeVar(ast::scope::Context* ty_cons) {
  type_store.push_back(Type{
      .typing_context_ = ty_cons,
      .as_var = TyVar{.id = type_store.size()},
  });

  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTypePtr(Type* underlying) {
  type_store.push_back(Type{.tag = types::TypeTag::TY_PTR,
                            .as_ptr = PtrType{.underlying = underlying}});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeFunType(Parameter* parameters, Type* result_type) {
  type_store.push_back(Type{.tag = TypeTag::TY_FUN,
                            .as_fun = FunType{
                                .parameters = parameters,
                                .result_type = result_type,
                            }});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTyApp(std::string_view name, Parameter* parameters) {
  type_store.push_back(
      Type{.tag = TypeTag::TY_APP,
           .as_tyapp = TyAppType{.name = name, .parameters = parameters}});
  return &type_store.back();
}

//////////////////////////////////////////////////////////////////////

Type* MakeTyCons(std::string_view name, std::vector<lex::Token> params,
                 Type* body, Type* kind, ast::scope::Context* context) {
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

Type* MakeStructType(Member* members) {
  type_store.push_back(Type{
      .tag = types::TypeTag::TY_STRUCT,
      .as_struct = StructTy{members},
  });
  return &type_store.back();
};

//////////////////////////////////////////////////////////////////////

Type* MakeSumType(Member* members) {
  type_store.push_back(Type{
      .tag = types::TypeTag::TY_SUM,
      .as_struct = StructTy{members},
  });
  return &type_store.back();
};

//////////////////////////////////////////////////////////////////////

}  // namespace types
