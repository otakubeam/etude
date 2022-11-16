#include <qbe/ir_emitter.hpp>
#include <qbe/gen_addr.hpp>
#include <qbe/gen_at.hpp>

namespace qbe {

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitVarDecl(VarDeclStatement* node) {
  // %.9 =l alloc4 4


  named_values_.insert({node->GetVarName(), id});
  auto size = GetTypeSize(node->value_->GetType());

  fmt::print("# declare {}\n", node->GetVarName());
  fmt::print("  %.{} =l alloc4 {}\n", id, size);

  GenAt at(*this, id);
  node->value_->Accept(&at);
  fmt::print("{}", at.Give());

  return_value = 0;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitAssignment(AssignmentStatement* node) {
  auto target_id = id_ += 1;
  GenAddr a(*this, target_id);
  node->target_->Accept(&a);
  fmt::print("{}", a.Give());

  auto size = GetTypeSize(node->value_->GetType());

  // TODO: Copy-loop

  fmt::print("  storew %.{} %.{}", Eval(node->value_), target_id);
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitFunDecl(FunDeclStatement* node) {
  auto mangled = std::string(node->GetFunctionName());
  mangled += types::Mangle(*node->type_);

  fmt::print("export function {} ${}(", "w", mangled);

  // function $test(:type.1 %s) {

  auto& arg_ty = node->type_->as_fun.param_pack;
  auto& formals = node->formals_;

  for (size_t i = 0; i < arg_ty.size(); i++) {
    auto id = id_ += 1;
    ids_.insert({formals[i].GetName(), id});
    fmt::print("{} %.{},", "w", id);
  }

  fmt::print(") {{ \n@start\n");
  auto v = Eval(node->body_);

  fmt::print("@ret\n");

  fmt::print("  ret %.{}\n}}\n\n", v);
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitFnCall(FnCallExpression* node) {
  // call $rt.memset(l %binding.5, l 0, l 8)
  fmt::print("# call {}\n", node->GetFunctionName());

  std::vector<int> ids;
  for (auto& a : node->arguments_) {
    // Is this a struct?
    if (measure_.IsStruct(a->GetType())) {
      ids.push_back(id_ += 1);
      GenAddr g(*this, ids.back());
      a->Accept(&g);
      fmt::print("{}", g.Give());
    } else {
      ids.push_back(Eval(a));
    }
  }

  auto mangled = std::string(node->GetFunctionName());
  mangled += types::Mangle(*node->callable_type_);

  fmt::print("  call ${}(", mangled);
  for (auto& i : ids) {
    fmt::print("{} {},", "l", i);
  }
  fmt::print(")\n");
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitIntrinsic(IntrinsicCall* node) {
  (void)node;
  std::abort();  // Unreachable
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitReturn(ReturnStatement* node) {
  auto qv = Eval(node->return_value_);
  fmt::print("  ret %.{}\n", qv);
  fmt::print("@block{}\n", id_ += 1);
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitYield(YieldStatement*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitExprStatement(ExprStatement* node) {
  Eval(node->expr_);
  return_value = 0;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitDeref(DereferenceExpression* node) {
  id_ += 1;
  fmt::print("  %.{} =w loadsw %.{}\n", id_, Eval(node->operand_));
  return_value = id_;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitAddressof(AddressofExpression* node) {
  auto res_id = id_ += 1;
  GenAddr gen{*this, res_id};

  node->operand_->Accept(&gen);
  fmt::print("{}", gen.Give());

  return_value = res_id;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitComparison(ComparisonExpression* node) {
  auto res_id = id_ += 1;
  auto l_id = Eval(node->left_);
  auto r_id = Eval(node->left_);

  switch (node->operator_.type) {
    case lex::TokenType::EQUALS:
      fmt::print("%.{} =w ceqw %.{}, %.{}", res_id, l_id, r_id);
      break;

    case lex::TokenType::LT:
      fmt::print("%.{} =w csltw %.{}, %.{}", res_id, l_id, r_id);
      break;

    case lex::TokenType::GE:
      fmt::print("%.{} =w csgew %.{}, %.{}", res_id, l_id, r_id);
      break;

    case lex::TokenType::LE:
      fmt::print("%.{} =w cslew %.{}, %.{}", res_id, l_id, r_id);
      break;

    case lex::TokenType::GT:
      fmt::print("%.{} =w csgtw %.{}, %.{}", res_id, l_id, r_id);
      break;

    default:
      std::abort();
  }

  return_value = res_id;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitBinary(BinaryExpression* node) {
  auto lid = Eval(node->left_);
  auto rid = Eval(node->right_);

  // Handle pointer arithmetic

  if (node->left_->GetType()->tag == types::TypeTag::TY_PTR) {
    auto ptr_type = node->left_->GetType();
    auto underlying = ptr_type->as_ptr.underlying;
    auto multiplier = GetTypeSize(underlying);
    if (multiplier != 1) {
      fmt::print("%.{} =w mul %.{}, %.{}", rid, rid, multiplier);
    }
  }

  auto res_id = id_ += 1;

  switch (node->operator_.type) {
    case lex::TokenType::PLUS:
      fmt::print("%.{} =w add %.{}, %.{}", res_id, lid, rid);
      break;

    case lex::TokenType::STAR:
      fmt::print("%.{} =w mul %.{}, %.{}", res_id, lid, rid);
      break;

    case lex::TokenType::MINUS:
      fmt::print("%.{} =w sub %.{}, %.{}", res_id, lid, rid);
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }

  return_value = res_id;
}

void IrEmitter::VisitUnary(UnaryExpression* node) {
  auto res_id = id_ += 1;

  switch (node->operator_.type) {
    case lex::TokenType::MINUS:
      fmt::print("%.{} =w neg %.{}\n",  //
                 res_id, Eval(node->operand_));
    case lex::TokenType::NOT:
      fmt::print("%.{} =w ceqw %.{}, 0\n",  //
                 res_id, Eval(node->operand_));
    default:
      std::abort();
  }

  return_value = res_id;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitIf(IfExpression* node) {
  auto true_id = id_ += 1;
  auto false_id = id_ += 1;
  auto join_id = id_ += 1;

  auto result_id = id_ += 1;

  fmt::print("#if-start\n");
  fmt::print("  %.{} =l alloc4 4\n", result_id);

  auto cond_id = Eval(node->condition_);
  fmt::print("  jnz %.{}, @true.{}, @false.{}\n", cond_id, true_id, false_id);

  fmt::print("@true.{}          \n", true_id);
  fmt::print("  storew %.{}, %.{}\n", Eval(node->true_branch_), result_id);
  fmt::print("  jmp @join.{}        \n", join_id);

  fmt::print("@false.{}         \n", false_id);
  fmt::print("  storew %.{}, %.{}\n", Eval(node->false_branch_), result_id);

  fmt::print("@join.{}          \n", join_id);
  return_value = result_id;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitNew(NewExpression* node) {
  // I can just `call` malloc here, no?

  (void)node;
  // Need runtime support
  std::abort();  // Unreachable
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitBlock(BlockExpression* node) {
  for (auto& statement : node->stmts_) {
    statement->Accept(this);
  }

  if (node->final_) {
    return_value = Eval(node->final_);
    return;
  }

  // Here I need to return something else

  std::abort();
  // return_value = Value{.tag = Value::INT, .int_v = 1};
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  auto size = GetTypeSize(node->GetType());
  auto res_id = id_ += 1;

  const auto ChooseAlignment = []() {
    return 4;
  };

  fmt::print("  %.{} =l alloc{} {}\n", res_id, ChooseAlignment(), size);

  GenAt a(*this, res_id);
  node->Accept(&a);

  fmt::print("{}", a.Give());
  return_value = res_id;
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitFieldAccess(FieldAccessExpression* node) {
  (void)node;
  std::abort();  // Unreachable
}

////////////////////////////////////////////////////////////////////

void IrEmitter::VisitTypecast(TypecastExpression* node) {
  // Unit -> Int -> Bool
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
      id_ += 1;

      fmt::print("  %.{} =w copy {}\n", id_,
                 std::get<int>(node->token_.sem_info));
      return_value = id_;
      break;

    case lex::TokenType::TRUE:
      id_ += 1;

      fmt::print("  %.{} =w copy {}\n", id_, 1);
      return_value = id_;
      break;

    case lex::TokenType::FALSE:
      id_ += 1;

      fmt::print("  %.{} =w copy {}\n", id_, 0);
      return_value = id_;
      break;

    case lex::TokenType::UNIT:

    case lex::TokenType::STRING:
      // Need to relocate string literal to the static data
      // std::get<std::string_view>(node->token_.sem_info);
      FMT_ASSERT(false, "Unimplemented!");

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

////////////////////////////////////////////////////////////////////

//  auto copy = [this](int src_id, int dst_id, size_t size) {
//    while (size >= 8) {
//      fmt::print("  %.{} =w loadl %.{}\n", dst_id, src_id);
//      fmt::print("  %.{} =w loadl %.{}\n", dst_id, src_id);
//      size -= 8;
//    }
//  };

void IrEmitter::VisitVarAccess(VarAccessExpression* node) {
  id_ += 1;
  Gen
  fmt::print("  %.{} =w loadsw %.{}\n", id_, ids_.at(node->GetName()));
  return_value = id_;
}

////////////////////////////////////////////////////////////////////

}  // namespace qbe
