#include <types/type.hpp>

#include <unordered_map>

namespace types {

//////////////////////////////////////////////////////////////////////

void Unify(Type* a, Type* b) {
  fmt::print("Unifying {} and {}\n\n", FormatType(*a), FormatType(*b));

  auto la = FindLeader(a);
  auto lb = FindLeader(b);

  if (la == lb) {
    return;
  }

  fmt::print("Their leaders: {} and {}\n", FormatType(*la), FormatType(*lb));

  // Always make the la be be a variable
  if (lb->tag == TypeTag::TY_VARIABLE) {
    std::swap(la, lb);
  }

  if (la->tag == TypeTag::TY_VARIABLE) {
    la->leader = lb;
    // la->as_variable.constraints + lb->as_variable.constraints
    return;
  }

  if (la->tag == lb->tag) {
    UnifyUnderlyingTypes(la, lb);
    return;
  }

  throw "Inference error: Tag mismatch";
}

//////////////////////////////////////////////////////////////////////

Type* FindLeader(Type* a) {
  FMT_ASSERT(a, "Fail");

  if (a->leader) {
    // fmt::print("Leader of {} is {}\n", a->id, a->leader->id);
    return a->leader = FindLeader(a->leader);
  } else {
    return a;
  }
}

// Type* FindLeader(Type* a) {
//   auto leader = a;
//   while (leader->leader) {
//     leader = leader->leader;
//   }
//   return leader;
// }

//////////////////////////////////////////////////////////////////////

void UnifyUnderlyingTypes(Type* a, Type* b) {
  // assert(la->tag == lb->tag);
  switch (a->tag) {
    case TypeTag::TY_PTR:
      Unify(a->as_ptr.underlying, b->as_ptr.underlying);
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
        Unify(a_mem[i].ty, b_mem[i].ty);
      }

      break;
    }

    case TypeTag::TY_FUN: {
      auto& pack = a->as_fun.param_pack;
      auto& pack2 = b->as_fun.param_pack;
      if (pack.size() != pack2.size()) {
        throw "Function unification size mismatch";
      }

      for (size_t i = 0; i < pack.size(); i++) {
        Unify(pack[i], pack2[i]);
      }

      Unify(a->as_fun.result_type, b->as_fun.result_type);
      break;
    }

    case TypeTag::TY_ALIAS:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_UNION:
      std::abort();

    default:
      break;
  }
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

    case TypeTag::TY_VARIABLE:
      l->tag = TypeTag::TY_PARAMETER;
      break;

    case TypeTag::TY_PARAMETER:
      // No-op
      break;

    case TypeTag::TY_ALIAS:
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

    case TypeTag::TY_FUN: {
      std::vector<Type*> args;

      auto& pack = l->as_fun.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        args.push_back(Instantinate(pack[i], map));
      }

      return MakeFunType(std::move(args),
                         Instantinate(l->as_fun.result_type, map));
    }

    case TypeTag::TY_ALIAS:
    case TypeTag::TY_STRUCT:
    case TypeTag::TY_UNION:
      std::abort();
      break;

    default:
      return l;  // Int, Bool, Unit, etc
  }
}

};  // namespace types
