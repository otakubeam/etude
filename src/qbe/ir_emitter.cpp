#include <qbe/ir_emitter.hpp>
#include <qbe/qbe_types.hpp>

#include <qbe/gen_match.hpp>
#include <qbe/gen_addr.hpp>
#include <qbe/gen_at.hpp>

#include <span>

namespace qbe {

////////////////////////////////////////////////////////////////////////////////

void IrEmitter::GenAddress(Expression* what, Value out) {
  GenAddr gen_addr(*this, out);
  what->Accept(&gen_addr);
};

void IrEmitter::GenAtAddress(Expression* what, Value where) {
  if (measure_.IsZST(what->GetType())) {
    return;
  }
  class GenAt gen_addr(*this, where);
  what->Accept(&gen_addr);
};

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitVarDecl(VarDeclStatement* node) {
  auto address = GenTemporary();
  named_values_.insert_or_assign(node->GetName(), address);

  auto [size, alignment] = SizeAlign(node->value_);
  fmt::print("# declare {}\n", node->GetName());

  fmt::print("  {} =l alloc{} {}\n",
             address.Emit(),  //
             alignment,       //
             size             //
  );

  // Gen at address handles big structures itself!

  GenAtAddress(node->value_, address);

  return_value = Value::None();
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitAssignment(AssignmentStatement* node) {
  auto out = GenTemporary();

  GenAddress(node->target_, out);
  GenAtAddress(node->value_, out);

  return_value = Value::None();
}

////////////////////////////////////////////////////////////////////

bool IsNomangle(Attribute* attr) {
  return attr && attr->FindAttr("nomangle");
}

bool IsTest(Attribute* attr) {
  return attr && attr->FindAttr("test");
}

void IrEmitter::VisitFunDecl(FunDeclStatement* node) {
}

////////////////////////////////////////////////////////////////////

bool IsFunctional(ast::scope::Symbol* symbol) {
  return symbol->sym_type == ast::scope::SymbolType::FUN ||
         symbol->sym_type == ast::scope::SymbolType::TRAIT_METHOD;
}

char GlobalFun(ast::scope::Symbol* symbol) {
  return IsFunctional(symbol) ? '$' : ' ';
}

void IrEmitter::VisitFnCall(FnCallExpression* node) {

  std::vector<Value> argument_temps;

  for(auto& argument : node->arguments_) {
    Value argument_temp = Eval(argument);
    argument_temps.push_back(argument_temp);
  }

  Value result_temp = GenTemporary();

  auto fun_name = node->GetFunctionName();
  

  fmt::print("  {} ={} call ${} (\n",
             result_temp.Emit(),  //
             ToQbeType(node->GetType()), //
             fun_name
  );

  for(int i = 0; i < argument_temps.size(); i++) {
    Value temp = argument_temps[i];
    fmt::print("{} {}", ToQbeType(node->arguments_[i]->GetType()), temp.Emit());
  }

  fmt::print(")\n");

  return_value = result_temp;

}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitIntrinsic(IntrinsicCall* node) {
  fmt::print("# Call intrinsic {}\n", node->GetFunctionName());

  switch (node->intrinsic) {
    case ast::elaboration::Intrinsic::PRINT:
      CallPrintf(node);
      break;

    case ast::elaboration::Intrinsic::ASSERT:
      CheckAssertion(node->arguments_[0]);
      break;

    default:
      std::abort();
  }

  return_value = Value::None();
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitReturn(ReturnStatement* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitYield(YieldStatement*) {
  FMT_ASSERT(false, "Unimplemented!");

  return_value = Value::None();
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitExprStatement(ExprStatement* node) {
  Eval(node->expr_);  // unused

  return_value = Value::None();
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitDeref(DereferenceExpression* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitAddressof(AddressofExpression* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitComparison(ComparisonExpression* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitBinary(BinaryExpression* node) {
}

void IrEmitter::VisitUnary(UnaryExpression* node) {
}

////////////////////////////////////////////////////////////////////

void PrintCopyInstruction(Value out, Value res, std::string_view assign) {
  if (res.tag != Value::NONE) {
    fmt::print("  {} = {} copy {}   \n", out.Emit(), assign, res.Emit());
  }
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitIf(IfExpression* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitMatch(MatchExpression* node) {
  auto assign = CopySuf(node->GetType());

  auto target = Eval(node->against_);

  // Materialize the value (see test match 36)
  auto materialized = GenTemporary();
  PrintCopyInstruction(materialized, target,
                       CopySuf(node->against_->GetType()));
  target = materialized;

  auto out = measure_.IsZST(node->GetType()) ? Value::None() : GenTemporary();
  auto end_id = id_ += 1;

  auto match_arm = id_ += 1;
  auto next_arm = id_ += 1;

  for (auto& [pat, expr] : node->patterns_) {
    auto lit = !measure_.IsCompound(node->against_->GetType());
    GenMatch match{*this, target, next_arm, lit};

    fmt::print("@match.{}          \n", match_arm);
    pat->Accept(&match);

    auto res = Eval(expr);
    PrintCopyInstruction(out, res, assign);

    fmt::print("  jmp @match_end.{}    \n", end_id);

    match_arm = next_arm;
    next_arm = id_ += 1;
  }

  fmt::print("@match.{}          \n", match_arm);
  CallAbort(node);
  fmt::print("@match_end.{}    \n", end_id);

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitNew(NewExpression* node) {
  auto out = GenTemporary();
  auto type_size = GetTypeSize(node->underlying_);

  auto size = GenTemporary();
  fmt::print("  {} =w copy {}\n", size.Emit(), type_size);

  if (node->allocation_size_) {
    auto alloc_size = Eval(node->allocation_size_);
    static constexpr auto fmt = "  {} =w mul {}, {}\n";
    fmt::print(fmt, size.Emit(), alloc_size.Emit(), type_size);
  }

  fmt::print("  {} =l call $malloc (w {}) \n", out.Emit(), size.Emit());

  if (node->initial_value_) {
    GenAtAddress(node->initial_value_, out);
  }

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitBlock(BlockExpression* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  auto out = GenTemporary();

  auto [size, alignment] = SizeAlign(node);
  fmt::print("  {} =l alloc{} {}\n", out.Emit(), alignment, size);

  GenAtAddress(node, out);
  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitFieldAccess(FieldAccessExpression* node) {
  auto addr = GenTemporary();

  if (measure_.IsZST(node->GetType())) {
    return_value = Value::None();
    return;
  }

  GenAddress(node, addr);

  if (measure_.IsCompound(node->GetType())) {
    return_value = addr;
    return;
  }

  auto out = GenTemporary();

  constexpr auto fmt = "  {} = {} load{} {}  \n";
  fmt::print(fmt, out.Emit(), ToQbeType(node->GetType()),
             LoadSuf(node->GetType()), addr.Emit());

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitTypecast(TypecastExpression* node) {
  // If the type is privimite, there is a set of built-in rules

  auto original = node->expr_->GetType();
  auto target = node->type_;

  if (GetTypeSize(original) == GetTypeSize(target)) {
    return_value = Eval(node->expr_);
    return;
  }

  if (original->tag == types::TypeTag::TY_UNIT &&
      target->tag == types::TypeTag::TY_PTR) {
    return_value = GenConstInt(0);
    return;
  }

  if (original->tag == types::TypeTag::TY_PTR &&
      target->tag == types::TypeTag::TY_BOOL) {
    // Subtyping: lowers automatically
    // https://c9x.me/compile/doc/il.html#Subtyping
    return_value = Eval(node->expr_);
    return;
  }

  if (original->tag == types::TypeTag::TY_CHAR &&
      target->tag == types::TypeTag::TY_INT) {
    auto cast = GenTemporary();
    fmt::print("  {} = w extub {}\n", cast.Emit(), Eval(node->expr_).Emit());
    return_value = cast;
    return;
  }

  (void)node;
  std::abort();  // Unreachable
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitLiteral(LiteralExpression* node) {
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitVarAccess(VarAccessExpression* node) {
}

////////////////////////////////////////////////////////////////////

}  // namespace qbe
