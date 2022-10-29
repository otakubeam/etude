#include <ast/scope/context.hpp>

namespace ast::scope {

void ScopeLayer::InsertSymbol(Symbol symbol) {
  symbols.push_back(std::move(symbol));
  symbol_map.insert({
      symbols.back().name,
      &symbols.back(),
  });
}

Context* Context::Find(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return this;
  }
  return parent == nullptr ? nullptr : parent->Find(name);
}

Context* Context::MakeNewScopeLayer(lex::Location loc, std::string_view name) {
  auto child = new Context{
      .name = name,
      .location = loc,
      .level = level + 1,
      .parent = this,
  };
  children.push_back(child);
  return child;
}

void Context::Print() {
  fmt::print(stderr, "[!] Context {} at {}, level {}\n", name,
             location.Format(), level);

  fmt::print(stderr, "Bindings: \n");
  for (auto& sym : bindings.symbols) {
    fmt::print(stderr, "{}:{}\n", sym.FormatSymbol(),
               types::FormatType(*sym.GetType()));
  }

  fmt::print(stderr, "\n");

  for (auto& ch : children) {
    ch->Print();
  }
}

using Type = types::Type;
using TypeTag = types::TypeTag;

Type* ConstructType(Type* ty, Context* ctx) {
  fmt::print("Constructing {}", (int)ty->tag);
  if (ty->tag != types::TypeTag::TY_APP) {
    return ConstructTypeRec(ty, ctx);
  }

  auto find = ctx->Find(ty->as_tyapp.name);
  fmt::print("Constructing {}\n", ty->as_tyapp.name.GetName());
  auto symbol = find->bindings.symbol_map.at(ty->as_tyapp.name);

  auto& pack = ty->as_tyapp.param_pack;
  auto& names = symbol->GetType()->as_tycons.param_pack;
  fmt::print("Constructed {}\n", ty->as_tyapp.name.GetName());

  if (pack.size() != names.size()) {
    throw "Instantination size mismatch";
  }

  ctx = ctx->MakeNewScopeLayer(ty->as_tyapp.name.location, ty->as_tyapp.name);

  for (size_t i = 0; i < pack.size(); i++) {
    ctx->bindings.InsertSymbol(Symbol{
        .sym_type = SymbolType::TYPE,
        .name = names[i],
        .as_type = {.type = new types::Type{.tag = TypeTag::TY_CONS,
                                            .as_tycons = {.name = names[i],
                                                          .body = pack[i]}}},
        .declared_at = symbol->declared_at,
    });
  }

  return ConstructType(symbol->GetType()->as_tycons.body, ctx);
}

Type* ConstructTypeRec(Type* ty, Context* ctx) {
  switch (ty->tag) {
    case TypeTag::TY_STRUCT: {
      auto& mems = ty->as_struct.first;
      std::vector<types::Member> instd_mems;

      for (auto& mem : mems) {
        instd_mems.push_back(types::Member{
            .field = mem.field,
            .ty = ConstructType(mem.ty, ctx),
        });
      }

      return types::MakeStructType(std::move(instd_mems));
    }

    case TypeTag::TY_PTR:
      return types::MakeTypePtr(ConstructType(ty->as_ptr.underlying, ctx));

    case TypeTag::TY_FUN: {
      std::vector<Type*> inst_pack;
      auto inst_res = ConstructType(ty->as_fun.result_type, ctx);

      auto& pack = ty->as_fun.param_pack;
      for (auto& p : pack) {
        inst_pack.push_back(ConstructType(p, ctx));
      }

      return types::MakeFunType(std::move(inst_pack), inst_res);
    }

    case TypeTag::TY_UNION:
      std::abort();  // Unimplemented

    case TypeTag::TY_KIND:
    case TypeTag::TY_CONS:
      std::abort();  // Should be unreachable

    case TypeTag::TY_PARAMETER:
    default:
      return ty;  // Int, Bool, Unit, etc
  }
}

}  // namespace ast::scope
