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
  Type::type_store.push_back(Type{.id = Type::type_store.size()});
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

}  // namespace types
// namespace types
