#include <types/constraints/solver.hpp>
#include <types/constraints/trait.hpp>

#include <unordered_map>

namespace types::constraints {

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::Unify(Type* a, Type* b) {
  if (a->tag == TypeTag::TY_NEVER || b->tag == TypeTag::TY_NEVER) {
    return true;  // Unify never with any type
  }

  auto la = FindLeader(a);
  auto lb = FindLeader(b);

  if (la == lb) {
    return true;
  }

  // Always make the la be be a variable
  if (lb->tag == TypeTag::TY_VARIABLE) {
    std::swap(la, lb);
  }

  if (la->tag == TypeTag::TY_VARIABLE) {
    la->leader = lb;

    // Do not merge constraints here, but find leader in solver

    return true;
  }

  if (la->tag == lb->tag) {
    return UnifyUnderlyingTypes(la, lb);
  }

  return false;
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::UnifyUnderlyingTypes(Type* a, Type* b) {
  switch (a->tag) {
    case TypeTag::TY_PTR:
      return Unify(a->as_ptr.underlying, b->as_ptr.underlying);

    case TypeTag::TY_STRUCT: {
      auto& a_mem = a->as_struct.first;
      auto& b_mem = b->as_struct.first;

      if (a_mem.size() != b_mem.size()) {
        return false;
      }

      for (size_t i = 0; i < a_mem.size(); i++) {
        if (!Unify(a_mem[i].ty, b_mem[i].ty)) {
          return false;
        }
      }

      break;
    }

    case TypeTag::TY_SUM: {
      auto& a_mem = a->as_sum.first;
      auto& b_mem = b->as_sum.first;

      if (a_mem.size() != b_mem.size()) {
        throw std::runtime_error{"Inference error: struct size mismatch"};
      }

      // Also don't forget to check the names!

      for (size_t i = 0; i < a_mem.size(); i++) {
        if (a_mem[i].field != b_mem[i].field) {
          throw std::runtime_error{"Inference error: sum field mismatch"};
        }

        if (!Unify(a_mem[i].ty, b_mem[i].ty)) {
          return false;
        }
      }

      break;
    }

    case TypeTag::TY_FUN: {
      auto& pack = a->as_fun.param_pack;
      auto& pack2 = b->as_fun.param_pack;

      if (pack.size() != pack2.size()) {
        return false;
      }

      for (size_t i = 0; i < pack.size(); i++) {
        if (!Unify(pack[i], pack2[i])) {
          return false;
        }
      }

      return Unify(a->as_fun.result_type, b->as_fun.result_type);
    }

    case TypeTag::TY_APP: {
      if (a->as_tyapp.name != b->as_tyapp.name) {
        while (auto new_a = ApplyTyconsLazy(a)) {
          fmt::print(stderr, "a ~ {}\n", FormatType(*new_a));
          a = new_a;
        }

        while (auto new_b = ApplyTyconsLazy(b)) {
          fmt::print(stderr, "b ~ {}\n", FormatType(*new_b));
          b = new_b;
        }

        return Unify(a, b);
      }

      auto& pack = a->as_tyapp.param_pack;
      auto& pack2 = b->as_tyapp.param_pack;

      for (size_t i = 0; i < pack.size(); i++) {
        if (!Unify(pack[i], pack2[i])) {
          return false;
        }
      }

      break;
    }

    case TypeTag::TY_CONS:
    case TypeTag::TY_KIND:
    case TypeTag::TY_NEVER:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_UNION:
      std::abort();

    default:
      break;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

void ConstraintSolver::Generalize(Type* ty) {
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

    case TypeTag::TY_SUM:
      for (auto& mem : l->as_sum.first) {
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

};  // namespace types::constraints
