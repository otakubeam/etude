#include <types/type.hpp>

#include <types/trait.hpp>
#include <unordered_map>

namespace types {

//////////////////////////////////////////////////////////////////////

void PushEqual(Type* a, Type* b, std::deque<Trait>& fill_queue) {
  fill_queue.push_back(Trait{
      .tag = TraitTags::TYPES_EQ,
      .types_equal = {.a = a, .b = b},
  });
}

//////////////////////////////////////////////////////////////////////

void Unify(Type* a, Type* b, std::deque<Trait>& fill_queue) {
  auto la = FindLeader(a);
  auto lb = FindLeader(b);

  if (la == lb) {
    return;
  }

  // Always make the la be be a variable
  if (lb->tag == TypeTag::TY_VARIABLE) {
    std::swap(la, lb);
  }

  if (la->tag == TypeTag::TY_VARIABLE) {
    la->leader = lb;

    // Do not merge constraints here, but find leader in solver

    return;
  }

  if (la->tag == lb->tag) {
    UnifyUnderlyingTypes(la, lb, fill_queue);
    return;
  }

  throw "Inference error: Tag mismatch";
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

void UnifyUnderlyingTypes(Type* a, Type* b, std::deque<Trait>& fill_queue) {
  // assert(la->tag == lb->tag);
  switch (a->tag) {
    case TypeTag::TY_PTR:
      PushEqual(a->as_ptr.underlying, b->as_ptr.underlying, fill_queue);
      break;

    case TypeTag::TY_STRUCT: {
      auto& a_mem = a->as_struct.first;
      auto& b_mem = b->as_struct.first;

      if (a_mem.size() != b_mem.size()) {
        throw "Inference error: struct size mismatch";
      }

      for (size_t i = 0; i < a_mem.size(); i++) {
        // Here I only look at the types
        // Should I also look at the field names?
        PushEqual(a_mem[i].ty, b_mem[i].ty, fill_queue);
      }

      break;
    }

    case TypeTag::TY_FUN: {
      auto& pack = a->as_fun.param_pack;
      auto& pack2 = b->as_fun.param_pack;

      if (pack.size() != pack2.size()) {
        throw std::runtime_error{"Function unification size mismatch"};
      }

      for (size_t i = 0; i < pack.size(); i++) {
        SetTyContext(pack[i], a->typing_context_);
        SetTyContext(pack2[i], b->typing_context_);

        PushEqual(pack[i], pack2[i], fill_queue);
      }

      SetTyContext(a->as_fun.result_type, a->typing_context_);
      SetTyContext(b->as_fun.result_type, b->typing_context_);

      PushEqual(a->as_fun.result_type, b->as_fun.result_type, fill_queue);
      break;
    }

    case TypeTag::TY_APP: {
      if (a->as_tyapp.name.GetName() != b->as_tyapp.name) {
        while (auto new_a = ApplyTyconsLazy(a)) {
          fmt::print("a ~ {}\n", FormatType(*new_a));
          a = new_a;
        }

        fmt::print("Checkpoint\n");

        while (auto new_b = ApplyTyconsLazy(b)) {
          fmt::print("b ~ {}\n", FormatType(*new_b));
          b = new_b;
        }

        PushEqual(a, b, fill_queue);
        return;

        throw std::runtime_error{"Different type constructors"};
      }

      auto& a_pack = a->as_tyapp.param_pack;
      auto& b_pack = b->as_tyapp.param_pack;

      for (size_t i = 0; i < a_pack.size(); i++) {
        PushEqual(a_pack[i], b_pack[i], fill_queue);
      }

      break;
    }

    case TypeTag::TY_CONS:
    case TypeTag::TY_KIND:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_UNION:
      std::abort();

    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

Type* SubstituteParameters(
    Type* subs, const std::unordered_map<std::string_view, Type*>& map) {
  switch (subs->tag) {
    case TypeTag::TY_PTR:
      return MakeTypePtr(SubstituteParameters(subs->as_ptr.underlying, map));

    case TypeTag::TY_STRUCT: {
      std::vector<Member> result;
      auto& pack = subs->as_struct.first;

      for (auto& p : pack) {
        result.push_back(
            Member{.field = p.field, .ty = SubstituteParameters(p.ty, map)});
      }

      return MakeStructType(std::move(result));
    }

    case TypeTag::TY_FUN: {
      std::vector<Type*> args;
      auto& pack = subs->as_fun.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        args.push_back(SubstituteParameters(pack[i], map));
      }

      Type* result = SubstituteParameters(subs->as_fun.result_type, map);
      return MakeFunType(std::move(args), result);
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

      return MakeTyApp(subs->as_tyapp.name, std::move(result));
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
    throw "Instantination size mismatch";
  }

  std::unordered_map<std::string_view, Type*> map;
  for (size_t i = 0; i < pack.size(); i++) {
    map.insert({names[i], pack[i]});
  }

  auto subs = SubstituteParameters(symbol->GetType()->as_tycons.body, map);
  SetTyContext(subs, ty->typing_context_);

  return subs;
}

//////////////////////////////////////////////////////////////////////

void Generalize(Type* ty) {
  auto l = FindLeader(ty);

  switch (l->tag) {
    case TypeTag::TY_PTR:
      Generalize(l->as_ptr.underlying);
      break;

    case TypeTag::TY_STRUCT:
      for (auto& mem : l->as_struct.first) {
        Generalize(mem.ty);
      }
      break;

    case TypeTag::TY_FUN: {
      auto& pack = l->as_fun.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        Generalize(pack[i]);
      }

      Generalize(l->as_fun.result_type);
      break;
    }

    case TypeTag::TY_APP:
      for (auto& mem : l->as_tyapp.param_pack) {
        Generalize(mem);
      }
      break;

    case TypeTag::TY_VARIABLE:
      l->tag = TypeTag::TY_PARAMETER;
      break;

    case TypeTag::TY_PARAMETER:
      // No-op
      break;

    case TypeTag::TY_CONS:
    case TypeTag::TY_KIND:
    case TypeTag::TY_UNION:
      std::abort();
    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

// Ty here is a type schema
using KnownParams = std::unordered_map<Type*, Type*>;
Type* Instantinate(Type* ty, KnownParams& map) {
  auto l = FindLeader(ty);

  switch (l->tag) {
    case TypeTag::TY_VARIABLE:
      return l;  // TODO: idk, when should I instantiate in recursive defs?

    case TypeTag::TY_PTR:
      return MakeTypePtr(Instantinate(l->as_ptr.underlying, map));

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

      return MakeTyApp(l->as_tyapp.name, std::move(args));
    }

    case TypeTag::TY_FUN: {
      std::vector<Type*> args;

      auto& pack = l->as_fun.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        args.push_back(Instantinate(pack[i], map));
      }

      return MakeFunType(std::move(args),
                         Instantinate(l->as_fun.result_type, map));
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

      return MakeStructType(std::move(args));
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

//////////////////////////////////////////////////////////////////////

};  // namespace types
