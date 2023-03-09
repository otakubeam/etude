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
    la->as_var.leader = lb;

    // Do not merge constraints here, but find leader in solver

    return true;
  }

  if (la->tag == lb->tag) {
    return UnifyUnderlyingTypes(la, lb);
  }

  return false;
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::UnifyStructs(StructTy* a, StructTy* b) {
  auto a_mem = a->members;
  auto b_mem = b->members;

  while (a_mem && b_mem) {
    if (a_mem->field != b_mem->field) {
      return false;
    }

    if (!Unify(a_mem->ty, b_mem->ty)) {
      return false;
    }

    a_mem = a_mem->next, b_mem = b_mem->next;
  }

  // If one list has ended, then structs are not equal
  return (a_mem || b_mem) ? false : true;
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::CompareParameterLists(Parameter* a, Parameter* b) {
  while (a && b) {
    if (!Unify(a->ty, b->ty)) {
      return false;
    }
    a = a->next, b = b->next;
  }

  return (a || b) ? false : true;
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::UnifyFunc(FunType* a, FunType* b) {
  return CompareParameterLists(a->parameters, b->parameters) &&
         Unify(a->result_type, b->result_type);
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::UnifyPtr(PtrType* a, PtrType* b) {
  return Unify(a->underlying, b->underlying);
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::UnifyTyApp(TyAppType* a, TyAppType* b) {
  return a->name == b->name &&
         CompareParameterLists(a->parameters, b->parameters);
}

//////////////////////////////////////////////////////////////////////

bool ConstraintSolver::UnifyUnderlyingTypes(Type* a, Type* b) {
  switch (a->tag) {
    case TypeTag::TY_PTR:
      return UnifyPtr(&a->as_ptr, &b->as_ptr);

    case TypeTag::TY_FUN:
      return UnifyFunc(&a->as_fun, &b->as_fun);

    case TypeTag::TY_APP:
      return UnifyTyApp(&a->as_tyapp, &b->as_tyapp);

    case TypeTag::TY_SUM:
    case TypeTag::TY_STRUCT:
      return UnifyStructs(&a->as_struct, &b->as_struct);

    case TypeTag::TY_CONS:
      // Later:
      // bind :: m(a) -> (a -> m(b)) -> m(b)
      //
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

    case TypeTag::TY_SUM:
    case TypeTag::TY_STRUCT:
      for (auto mem = l->as_struct.members; mem; mem = mem->next) {
        Generalize(mem->ty);
      }
      break;

    case TypeTag::TY_FUN: {
      for (auto p = l->as_fun.parameters; p; p = p->next) {
        Generalize(p->ty);
      }

      Generalize(l->as_fun.result_type);
      break;
    }

    case TypeTag::TY_APP:
      for (auto p = l->as_tyapp.parameters; p; p = p->next) {
        Generalize(p->ty);
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
