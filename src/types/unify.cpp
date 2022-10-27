#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

void Unify(Type* a, Type* b) {
  fmt::print("Unifying {} and {}\n\n", FormatType(*a), FormatType(*b));

  auto la = FindLeader(a);
  auto lb = FindLeader(b);

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

};  // namespace types
