#include <ast/elaboration/quantify_types.hpp>
#include <ast/declarations.hpp>
#include <ast/patterns.hpp>
#include <lex/token.hpp>

namespace ast::elaboration {

//////////////////////////////////////////////////////////////////////

using types::MakeTypeVar;
using types::Type;
using types::TypeTag;

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

      ty->tag = types::TypeTag::TY_PARAMETER;
      ty->as_var.leader = symbol->as_type.cons;
    }

  } else {
    //
    // If the symbol is unknown in this context, treat is
    // as universally quantified type
    //
    auto loc = ty->typing_context_->location;

    fmt::print(stderr, "Defining generic {} at {}\n", name, loc.Format());

    auto par = ty->as_var.leader = MakeTypeVar(ty->typing_context_);
    auto generic_sym = ast::scope::MakeGenericSymbol(name, par, loc);

    ty->as_var.leader = par;
    ty->tag = types::TypeTag::TY_PARAMETER;
    ty->typing_context_->InsertSymbol(generic_sym);
  }
}

//////////////////////////////////////////////////////////////////////

void Traverse(Type* ty) {
  ty = FindLeader(ty);
  DefineGenerics(ty);

  switch (ty->tag) {
    case TypeTag::TY_APP:
      for (auto a = ty->as_tyapp.parameters; a; a = a->next) {
        Traverse(a->ty);
      }
      break;

    case TypeTag::TY_FUN:
      for (auto a = ty->as_fun.parameters; a; a = a->next) {
        Traverse(a->ty);
      }

      Traverse(ty->as_fun.result_type);
      break;

    case TypeTag::TY_PTR:
      Traverse(ty->as_ptr.underlying);
      break;

    case TypeTag::TY_SUM:
    case TypeTag::TY_STRUCT:
      for (auto a = ty->as_struct.members; a; a = a->next) {
        Traverse(a->ty);
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

void QuantifyTypes::VisitTypeDecl(TypeDeclaration*) {
  // No-op
}

//////////////////////////////////////////////////////////////////////

void QuantifyTypes::VisitVarDecl(VarDeclaration* node) {
  if (!node->annotation_) {
    return;
  }

  Traverse(node->annotation_);
}

//////////////////////////////////////////////////////////////////////

void QuantifyTypes::VisitFunDecl(FunDeclaration* node) {
  if (node->type_) {
    Traverse(node->type_);
  }

  if (node->body_) {
    node->body_->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void QuantifyTypes::VisitTraitDecl(TraitDeclaration* node) {
  for (auto decl : node->assoc_items_) {
    decl->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void QuantifyTypes::VisitImplDecl(ImplDeclaration* node) {
  for (auto decl : node->assoc_items_) {
    decl->Accept(this);
  }
}

//////////////////////////////////////////////////////////////////////

void QuantifyTypes::VisitYield(YieldExpression* node) {
  node->yield_value_->Accept(this);
}

void QuantifyTypes::VisitReturn(ReturnExpression* node) {
  node->return_value_->Accept(this);
}

void QuantifyTypes::VisitAssign(AssignExpression* node) {
  node->value_->Accept(this);
  node->target_->Accept(this);
}

void QuantifyTypes::VisitSeqExpr(SeqExpression* node) {
  node->expr_->Accept(this);
}

//////////////////////////////////////////////////////////////////////

void QuantifyTypes::VisitComparison(ComparisonExpression*) {
}

void QuantifyTypes::VisitBinary(BinaryExpression*) {
}

void QuantifyTypes::VisitUnary(UnaryExpression*) {
}

void QuantifyTypes::VisitDeref(DereferenceExpression*) {
}

void QuantifyTypes::VisitAddressof(AddressofExpression*) {
}

void QuantifyTypes::VisitIf(IfExpression*) {
}

void QuantifyTypes::VisitMatch(MatchExpression*) {
}

void QuantifyTypes::VisitNew(NewExpression* node) {
  Traverse(node->underlying_);
}

void QuantifyTypes::VisitLet(LetExpression*) {
}

void QuantifyTypes::VisitBlock(BlockExpression*) {
}

void QuantifyTypes::VisitIndex(IndexExpression*) {
}

void QuantifyTypes::VisitFnCall(FnCallExpression*) {
}

void QuantifyTypes::VisitIntrinsic(IntrinsicCall*) {
}

void QuantifyTypes::VisitCompoundInitalizer(CompoundInitializerExpr*) {
}

void QuantifyTypes::VisitFieldAccess(FieldAccessExpression*) {
}

void QuantifyTypes::VisitVarAccess(VarAccessExpression*) {
}

void QuantifyTypes::VisitLiteral(LiteralExpression*) {
}

void QuantifyTypes::VisitTypecast(TypecastExpression* node) {
  Traverse(node->type_);
  node->expr_->Accept(this);
}

}  // namespace ast::elaboration
