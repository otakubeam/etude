#include <types/constraints/expand/expand.hpp>
#include <types/constraints/solver.hpp>

#include <ast/declarations.hpp>
#include <ast/patterns.hpp>
#include <lex/token.hpp>

namespace types::constraints {

//////////////////////////////////////////////////////////////////////

void DefineGenerics(Type* ty) {
  if (ty->tag != TypeTag::TY_APP) {
    return;
  }

  auto name = ty->as_tyapp.name;

  if (auto symbol = ty->typing_context_->RetrieveSymbol(name)) {
    //
    // If the symbol has already been defined as generic,
    // then use its definition
    //
    if (symbol->sym_type == ast::scope::SymbolType::GENERIC) {
      fmt::print(stderr, "Using generic {}\n", name);

      ty->leader = symbol->as_type.cons;
    }

  } else {
    //
    // If the symbol is unknown in this context, treat is
    // as universally quantified type
    //
    auto loc = ty->typing_context_->location;

    fmt::print(stderr, "Defining generic {} at {}\n", name, loc.Format());

    ty->leader = MakeTypeVar(ty->typing_context_);

    auto generic_sym = ast::scope::MakeGenericSymbol(name, ty->leader, loc);
    ty->typing_context_->InsertSymbol(generic_sym);
  }
}

//////////////////////////////////////////////////////////////////////

void Traverse(Type* ty) {
  ty = FindLeader(ty);
  DefineGenerics(ty);

  switch (ty->tag) {
    case TypeTag::TY_APP:
      for (auto& a : ty->as_tyapp.param_pack) {
        Traverse(a);
      }
      break;

    case TypeTag::TY_FUN:
      for (auto& a : ty->as_fun.param_pack) {
        Traverse(a);
      }

      Traverse(ty->as_fun.result_type);
      break;

    case TypeTag::TY_PTR:
      Traverse(ty->as_ptr.underlying);
      break;

    case TypeTag::TY_STRUCT:
      for (auto& a : ty->as_struct.first) {
        Traverse(a.ty);
      }
      break;

    case TypeTag::TY_SUM:
      for (auto& a : ty->as_sum.first) {
        Traverse(a.ty);
      }
      break;

    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_PARAMETER:
    case TypeTag::TY_INT:
    case TypeTag::TY_BOOL:
    case TypeTag::TY_CHAR:
    case TypeTag::TY_UNIT:
    case TypeTag::TY_BUILTIN:
    case TypeTag::TY_KIND:
      return;

    case TypeTag::TY_CONS:
    case TypeTag::TY_UNION:
    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitTypeDecl(TypeDeclaration*) {
  // No-op
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitVarDecl(VarDeclaration* node) {
  if (!node->annotation_) {
    return;
  }

  Traverse(node->annotation_);
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitFunDecl(FunDeclaration* node) {
  if (node->type_) {
    Traverse(node->type_);
  }

  if (node->body_) {
    node->body_->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitTraitDecl(TraitDeclaration* node) {
  for (auto decl : node->assoc_items_) {
    decl->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitImplDecl(ImplDeclaration* node) {
  for (auto decl : node->assoc_items_) {
    decl->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitYield(YieldExpression* node) {
  node->yield_value_->Accept(this);
}

void ExpandTypeVariables::VisitReturn(ReturnExpression* node) {
  node->return_value_->Accept(this);
}

void ExpandTypeVariables::VisitAssign(AssignExpression* node) {
  node->value_->Accept(this);
  node->target_->Accept(this);
}

void ExpandTypeVariables::VisitSeqExpr(SeqExpression* node) {
  node->expr_->Accept(this);
}

//////////////////////////////////////////////////////////////////////

void ExpandTypeVariables::VisitComparison(ComparisonExpression*) {
}

void ExpandTypeVariables::VisitBinary(BinaryExpression*) {
}

void ExpandTypeVariables::VisitUnary(UnaryExpression*) {
}

void ExpandTypeVariables::VisitDeref(DereferenceExpression*) {
}

void ExpandTypeVariables::VisitAddressof(AddressofExpression*) {
}

void ExpandTypeVariables::VisitIf(IfExpression*) {
}

void ExpandTypeVariables::VisitMatch(MatchExpression*) {
}

void ExpandTypeVariables::VisitNew(NewExpression* node) {
  Traverse(node->underlying_);
}

void ExpandTypeVariables::VisitLet(LetExpression*) {
}

void ExpandTypeVariables::VisitBlock(BlockExpression*) {
}

void ExpandTypeVariables::VisitIndex(IndexExpression*) {
}

void ExpandTypeVariables::VisitFnCall(FnCallExpression*) {
}

void ExpandTypeVariables::VisitIntrinsic(IntrinsicCall*) {
}

void ExpandTypeVariables::VisitCompoundInitalizer(CompoundInitializerExpr*) {
}

void ExpandTypeVariables::VisitFieldAccess(FieldAccessExpression*) {
}

void ExpandTypeVariables::VisitVarAccess(VarAccessExpression*) {
}

void ExpandTypeVariables::VisitLiteral(LiteralExpression*) {
}

void ExpandTypeVariables::VisitTypecast(TypecastExpression* node) {
  Traverse(node->type_);
  node->expr_->Accept(this);
}

}  // namespace types::constraints
