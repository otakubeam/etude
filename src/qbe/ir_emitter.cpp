#include <qbe/ir_emitter.hpp>
#include <qbe/qbe_types.hpp>
#include <qbe/gen_addr.hpp>
#include <qbe/gen_at.hpp>

#include <span>

namespace qbe {

////////////////////////////////////////////////////////////////////////////////

void IrEmitter::GenAddress(Expression* what, Value out) {
  GenAddr gen_addr(*this, out);
  what->Accept(&gen_addr);
  fmt::print("{}", gen_addr.Give());
};

void IrEmitter::GenAtAddress(Expression* what, Value where) {
  GenAt(what, where);
  // GenAt gen_at(*this, where);
  // what->Accept(&gen_at);
  // fmt::print("{}", gen_at.Give());
};

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitVarDecl(VarDeclStatement* node) {
  auto address = GenTemporary();
  named_values_.insert_or_assign(node->GetVarName(), address);

  auto type = node->value_->GetType();
  auto size = GetTypeSize(type);
  auto alignment = measure_.MeasureAlignment(type);

  fmt::print("# declare {}\n", node->GetVarName());
  fmt::print("  {} =l alloc{} {}\n", address.Emit(), alignment, size);

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

void IrEmitter::VisitFunDecl(FunDeclStatement* node) {
  auto mangled = std::string(node->GetFunctionName());

  if (!node->GetFunctionName().starts_with("main")) {
    mangled += types::Mangle(*node->type_);
  }

  auto qbe_ty = ToQbeType(node->type_->as_fun.result_type);
  fmt::print("export function {} ${} (", qbe_ty, mangled);

  auto& arg_ty = node->type_->as_fun.param_pack;
  auto& formals = node->formals_;

  for (size_t i = 0; i < arg_ty.size(); i++) {
    auto t = GenParam();
    named_values_.insert_or_assign(formals[i].GetName(), t);

    fmt::print("{} {},", ToQbeType(arg_ty[i]), t.Emit());
  }

  fmt::print(") {{ \n");
  fmt::print("@start\n");

  auto out = Eval(node->body_);

  fmt::print("@ret\n");
  fmt::print("  ret {}\n", out.Emit());
  fmt::print("}}\n\n");
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitFnCall(FnCallExpression* node) {
  auto out = GenTemporary();

  // %out = call $rt.memset(l %binding.5, l 0, l 8)
  fmt::print("# call {}\n", node->GetFunctionName());

  struct Arg {
    Value v;
    std::string qbe_ty;
  };

  std::vector<Arg> args;
  for (auto& a : node->arguments_) {
    args.push_back(Arg{Eval(a), ToQbeType(a->GetType())});
  }

  auto mangled = std::string(node->GetFunctionName());
  mangled += types::Mangle(*node->callable_type_);

  auto result_ty = ToQbeType(node->GetType());
  fmt::print("  {} = {} call ${}(", out.Emit(), result_ty, mangled);

  for (auto& i : args) {
    fmt::print("{} {},", i.qbe_ty, i.v.Emit());
  }

  fmt::print(")\n");

  return_value = out;
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
  auto returning = Eval(node->return_value_);

  fmt::print("  ret {}\n", returning.Emit());
  fmt::print("@block{}\n", id_ += 1);

  return_value = Value::None();
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
  // This only works for small stuff!

  auto temp = GenTemporary();
  fmt::print("  {} =w load{} {}\n",  //
             temp.Emit(), LoadSuf(node->GetType()),
             Eval(node->operand_).Emit());

  return_value = temp;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitAddressof(AddressofExpression* node) {
  auto out = GenTemporary();
  GenAddress(node->operand_, out);
  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitComparison(ComparisonExpression* node) {
  auto out = GenTemporary();
  auto left = Eval(node->left_);
  auto right = Eval(node->right_);

  switch (node->operator_.type) {
    case lex::TokenType::EQUALS:
      fmt::print("  {} =w ceq{} {}, {}\n",  //
                 out.Emit(), ToQbeType(node->left_->GetType()), left.Emit(),
                 right.Emit());
      break;

    case lex::TokenType::LT:
      fmt::print("  {} =w csltw {}, {}\n",  //
                 out.Emit(), left.Emit(), right.Emit());
      break;

    case lex::TokenType::GE:
      fmt::print("  {} =w csgew {}, {}\n",  //
                 out.Emit(), left.Emit(), right.Emit());
      break;

    case lex::TokenType::LE:
      fmt::print("  {} =w cslew {}, {}\n",  //
                 out.Emit(), left.Emit(), right.Emit());
      break;

    case lex::TokenType::GT:
      fmt::print("  {} =w csgtw {}, {}\n",  //
                 out.Emit(), left.Emit(), right.Emit());
      break;

    default:
      std::abort();
  }

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitBinary(BinaryExpression* node) {
  auto out = GenTemporary();
  auto left = Eval(node->left_);
  auto right = Eval(node->right_);

  // Handle pointer arithmetic

  if (node->left_->GetType()->tag == types::TypeTag::TY_PTR) {
    auto ptr_type = node->left_->GetType();

    auto underlying = ptr_type->as_ptr.underlying;
    auto multiplier = GetTypeSize(underlying);

    auto temp = GenTemporary();
    fmt::print("  {} =l extuw {}\n", temp.Emit(), right.Emit());

    if (multiplier != 1) {
      fmt::print("  {} =l mul {}, {}\n",  //
                 temp.Emit(), temp.Emit(), multiplier);
    }

    right = temp;
  }

  switch (node->operator_.type) {
    case lex::TokenType::PLUS:
      fmt::print("  {} ={} add {}, {}\n",  //
                 out.Emit(), ToQbeType(node->GetType()), left.Emit(),
                 right.Emit());
      break;

    case lex::TokenType::STAR:
      fmt::print("  {} =w mul {}, {}\n",  //
                 out.Emit(), left.Emit(), right.Emit());
      break;

    case lex::TokenType::MINUS:
      fmt::print("  {} =w sub {}, {}\n",  //
                 out.Emit(), left.Emit(), right.Emit());
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }

  return_value = out;
}

void IrEmitter::VisitUnary(UnaryExpression* node) {
  auto out = GenTemporary();

  switch (node->operator_.type) {
    case lex::TokenType::MINUS:
      fmt::print("  {} =w neg {}\n",  //
                 out.Emit(), Eval(node->operand_).Emit());
      break;

    case lex::TokenType::NOT:
      fmt::print("  {} =w ceqw {}, 0\n",  //
                 out.Emit(), Eval(node->operand_).Emit());
      break;

    default:
      std::abort();
  }

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitIf(IfExpression* node) {
  auto true_id = id_ += 1;
  auto false_id = id_ += 1;
  auto join_id = id_ += 1;

  auto out = GenTemporary();
  auto condition = Eval(node->condition_);

  fmt::print("#if-start\n");
  fmt::print("  jnz {}, @true.{}, @false.{}\n",  //
             condition.Emit(), true_id, false_id);

  fmt::print("@true.{}          \n", true_id);
  auto true_v = Eval(node->true_branch_);

  fmt::print("  {} =w copy {}   \n", out.Emit(), true_v.Emit());
  fmt::print("  jmp @join.{}    \n", join_id);

  fmt::print("@false.{}         \n", false_id);
  auto false_v = Eval(node->false_branch_);

  fmt::print("  {} =w copy {}   \n", out.Emit(), false_v.Emit());
  fmt::print("@join.{}          \n", join_id);

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
    fmt::print("  {} =w mul {}, {}\n",  //
               size.Emit(), alloc_size.Emit(), type_size);
  }

  fmt::print("  {} =l call $malloc (w {})\n", out.Emit(), size.Emit());

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitBlock(BlockExpression* node) {
  // Perhaps I can allocate storage for this
  // and generate all yielded values there
  // Maybe this will be optimized out

  for (auto& statement : node->stmts_) {
    statement->Accept(this);
  }

  if (node->final_) {
    return_value = Eval(node->final_);
    return;
  }

  // Here I need to return something else

  return_value = Value{
      .tag = Value::CONST_INT,
      .value = 0,
  };
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  auto out = GenTemporary();
  auto size = GetTypeSize(node->GetType());
  auto alignment = measure_.MeasureAlignment(node->GetType());

  fmt::print("  {} =l alloc{} {}\n", out.Emit(), alignment, size);

  GenAtAddress(node, out);

  return_value = out;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitFieldAccess(FieldAccessExpression* node) {
  auto out = GenTemporary();
  auto field_name = node->GetFieldName();

  auto struct_type = node->struct_expression_->GetType();
  auto offset = measure_.MeasureFieldOffset(struct_type, field_name);

  GenAddress(node->struct_expression_, out);

  fmt::print("  {} =l add {}, {} \n", out.Emit(), out.Emit(), offset);

  if (measure_.IsStruct(node->GetType())) {
    return_value = out;
    return;
  }

  auto temp = GenTemporary();

  fmt::print("  {} = {} load{} {} \n",  //
             temp.Emit(), ToQbeType(node->GetType()), LoadSuf(node->GetType()),
             out.Emit());

  return_value = temp;
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

  if (original->tag == types::TypeTag::TY_PTR &&
      target->tag == types::TypeTag::TY_BOOL) {
    // Subtyping: lowers automatically
    // https://c9x.me/compile/doc/il.html#Subtyping
    return_value = Eval(node->expr_);
    return;
  }

  (void)node;
  std::abort();  // Unreachable
}

void IrEmitter::VisitTypeDecl(TypeDeclStatement*) {
  std::abort();  // Unreachable
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitLiteral(LiteralExpression* node) {
  switch (node->token_.type) {
    case lex::TokenType::NUMBER:
      return_value = GenConstInt(std::get<int>(node->token_.sem_info));
      break;

    case lex::TokenType::TRUE:
      return_value = GenConstInt(1);
      break;

    case lex::TokenType::FALSE:
      return_value = GenConstInt(0);
      break;

    case lex::TokenType::STRING:
      // section ".data.strdata.0"
      // data $strdata.0 = { b "fowiejf" }

      return_value =
          Value{.tag = Value::GLOBAL,
                .name = fmt::format("strdata.{}", string_literals_.size())};

      // section ".data.lit"
      // data $lit = { l $strdata.0, l 7, l 7 }

      string_literals_.push_back(
          std::get<std::string_view>(node->token_.sem_info));
      break;

    case lex::TokenType::UNIT:
      return_value = GenConstInt(0);
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitVarAccess(VarAccessExpression* node) {
  auto out = GenTemporary();
  auto location = named_values_.at(node->GetName());

  auto eq_type = ToQbeType(node->GetType());
  auto load_suf = LoadSuf(node->GetType());

  switch (location.tag) {
    // Don't need to load params
    case Value::PARAM:
      return_value = location;
      return;

      // But do need to load locals
    case Value::TEMPORARY:
      fmt::print("  {} = {} load{} {}\n",  //
                 out.Emit(), eq_type, load_suf, location.Emit());
      break;

    default:
      std::abort();
  }

  return_value = out;
}

////////////////////////////////////////////////////////////////////

}  // namespace qbe