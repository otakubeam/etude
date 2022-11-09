#include <vm/codegen/compiler.hpp>
#include <vm/rt/native_table.hpp>

namespace vm::codegen {

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarDecl(VarDeclStatement* node) {
  // Generate code to place value on stack

  node->value_->Accept(this);

  //
  // Set a flag that I am building this value
  //
  // This should make sp_ display this information
  //

  debug::DebugInfo debug_info{
      .location = node->GetLocation(),
      .dbg_instr = {{.var_name = std::string{node->GetVarName()},
                     .type_name = node->value_->GetType()->Format()}}};

  translator_->LastDie() = std::move(debug_info);

  // Infrom FrameTranslator about this location

  auto name = node->GetVarName();
  auto size = GetValueSize(node->value_);
  current_frame_->AddLocal(name, size);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAssignment(AssignmentStatement* node) {
  node->value_->Accept(this);
  auto instrs = GenAddress(node->target_);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();

  TranslateInstruction({
      .type = InstrType::STORE,
      .arg = GetValueSize(node->target_),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFunDecl(FunDeclStatement* node) {
  //
  // Craft initial stack layout (parameters, registers)
  //
  current_frame_ = new FrameTranslator{node, measure_};

  auto mangled = std::string(node->GetFunctionName());
  mangled += types::Mangle(*node->type_);

  node->body_->Accept(this);

  TranslateInstruction({.type = InstrType::RET_FN});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFnCall(FnCallExpression* node) {
  uint8_t size = 0;
  //
  // Place arguments in reverse order
  //
  for (int i = node->arguments_.size() - 1; i >= 0; i -= 1) {
    node->arguments_[i]->Accept(this);

    // For TailCallElmination

    size += GetValueSize(node->arguments_[i]);

    // Emit debug information (for now only the type)

    translator_->LastDie() = {
        .location = node->GetLocation(),
        .dbg_instr = {{.type_name = node->arguments_[i]->GetType()->Format()}}};
  }

  // Branch direct / indirect

  if (auto mb_offset = current_frame_->LookupOffset(node->GetFunctionName())) {
    EmitMemFetch(*mb_offset);
    TranslateInstruction({
        .type = vm::InstrType::INDIRECT_CALL,
    });
  } else if (node->is_tail_call_) {
    TranslateInstruction({
        .type = vm::InstrType::TAIL_CALL,
        .arg = size,
    });
  } else {
    TranslateInstruction({
        .type = vm::InstrType::CALL_FN,
        .fn_name = std::string{node->GetFunctionName()} +
                   types::Mangle(*node->callable_type_),
    });
  }

  // Don't forget to clean up the stack

  TranslateInstruction({
      .type = vm::InstrType::FIN_CALL,
      .arg = size,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitIntrinsic(IntrinsicCall* node) {
  for (int i = node->arguments_.size() - 1; i >= 0; i -= 1) {
    node->arguments_[i]->Accept(this);
  }

  TranslateInstruction(FatInstr::MakePushInt(node->arguments_.size()));
  TranslateInstruction({
      .type = vm::InstrType::NATIVE_CALL,
      .arg = (uint8_t)node->intrinsic,
      .debug_info = {.location = node->GetLocation()},
  });

  return;
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitReturn(ReturnStatement* node) {
  if (auto fn_call = node->return_value_->as<FnCallExpression>()) {
    std::string mangled = std::string(fn_call->GetFunctionName());
    mangled += types::Mangle(*types::FindLeader(fn_call->callable_type_));

    if (mangled == translator_->GetFunctionName()) {
      fn_call->is_tail_call_ = true;
    }
  }

  node->return_value_->Accept(this);

  TranslateInstruction({
      .type = InstrType::RET_FN,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitYield(YieldStatement*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitExprStatement(ExprStatement* node) {
  node->expr_->Accept(this);

  auto size = GetValueSize(node->expr_);

  for (auto i = 0; i < size; i++) {
    TranslateInstruction({.type = InstrType::POP_STACK});
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitDeref(DereferenceExpression* node) {
  // This is a path for rvalue conversion
  // Those who want lvalue (only '=`?) should just call GenAddress directly
  auto instrs = GenAddress(node);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();

  TranslateInstruction({
      .type = InstrType::LOAD,
      .arg = GetValueSize(node),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAddressof(AddressofExpression* node) {
  // TODO: assert that operand is not a deref expression

  auto instrs = GenAddress(node->operand_);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitComparison(ComparisonExpression* node) {
  InstrType type;

  node->left_->Accept(this);
  node->right_->Accept(this);

  switch (node->operator_.type) {
    case lex::TokenType::EQUALS:
      type = vm::InstrType::CMP_EQ;
      break;

    case lex::TokenType::LT:
      type = vm::InstrType::CMP_LESS;
      break;

    case lex::TokenType::GE:
      type = vm::InstrType::CMP_GE;
      break;

    case lex::TokenType::LE:
      type = vm::InstrType::CMP_LE;
      break;

    case lex::TokenType::GT:
      type = vm::InstrType::CMP_GREATER;
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }

  TranslateInstruction(FatInstr{.type = type});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitBinary(BinaryExpression* node) {
  InstrType type;

  node->left_->Accept(this);
  node->right_->Accept(this);

  if (node->left_->GetType()->tag == types::TypeTag::TY_PTR) {
    auto ptr_type = node->left_->GetType();
    auto underlying = ptr_type->as_ptr.underlying;
    auto multiplier = GetTypeSize(underlying);
    if (multiplier != 1) {
      TranslateInstruction(FatInstr::MakePushInt(multiplier));
      TranslateInstruction(FatInstr{.type = InstrType::MUL});
    }
  }

  switch (node->operator_.type) {
    case lex::TokenType::PLUS:
      type = InstrType::ADD;
      break;

    case lex::TokenType::STAR:
      type = InstrType::MUL;
      break;

    case lex::TokenType::MINUS:
      type = InstrType::SUBTRACT;
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }

  TranslateInstruction(FatInstr{.type = type});
}

void Compiler::VisitUnary(UnaryExpression*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitIf(IfExpression* node) {
  node->condition_->Accept(this);

  auto backpatch_to_false = translator_->GetLabel();

  TranslateInstruction({.type = InstrType::JUMP_IF_FALSE});

  // True branch

  node->true_branch_->Accept(this);

  auto backpatch_to_end = translator_->GetLabel();

  TranslateInstruction({.type = InstrType::JUMP});

  // False branch

  translator_->BackpatchLabel(backpatch_to_false);

  node->false_branch_->Accept(this);

  translator_->BackpatchLabel(backpatch_to_end);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitNew(NewExpression* node) {
  if (auto dyn_size = node->allocation_size_) {
    dyn_size->Accept(this);
  } else {
    TranslateInstruction(FatInstr::MakePushInt(GetTypeSize(node->underlying_)));
  }

  TranslateInstruction({.type = InstrType::ALLOC});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitBlock(BlockExpression* node) {
  for (auto& statement : node->stmts_) {
    statement->Accept(this);
  }

  if (node->final_) {
    node->final_->Accept(this);
    return;
  }

  // Why not just pop the last instr?
  TranslateInstruction({.type = InstrType::PUSH_UNIT});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitCompoundInitalizer(CompoundInitializerExpr* node) {
  auto size = GetValueSize(node);

  int16_t filled = 0;
  for (auto& mem : node->initializers_) {
    auto field_offset = GetFieldOffset(node->GetType(), mem.field);

    // Fill space in between designated fields

    while (filled < field_offset) {
      TranslateInstruction({.type = InstrType::PUSH_UNIT});
      filled += 1;
    }

    // Fill designated value

    mem.init->Accept(this);
    filled += GetValueSize(mem.init);
  }

  // Fill the rest of the space

  while (filled < size) {
    TranslateInstruction({.type = InstrType::PUSH_UNIT});
    filled += 1;
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFieldAccess(FieldAccessExpression* node) {
  auto instrs = GenAddress(node);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();

  TranslateInstruction({
      .type = InstrType::LOAD,
      .arg = GetValueSize(node),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitTypecast(TypecastExpression* node) {
  // No-op for now, there is a lot of logic
  node->expr_->Accept(this);
}

void Compiler::VisitTypeDecl(TypeDeclStatement*) {
  std::abort();  // Unreachable
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitLiteral(LiteralExpression* lit) {
  switch (lit->token_.type) {
    case lex::TokenType::NUMBER:
      TranslateInstruction(
          FatInstr::MakePushInt(std::get<int>(lit->token_.sem_info)));
      break;

    case lex::TokenType::UNIT:
      TranslateInstruction({.type = InstrType::PUSH_UNIT});
      break;

    case lex::TokenType::TRUE:
      TranslateInstruction({.type = InstrType::PUSH_TRUE});
      break;

    case lex::TokenType::FALSE:
      TranslateInstruction({.type = InstrType::PUSH_FALSE});
      break;

    case lex::TokenType::STRING:
      FMT_ASSERT(false, "Unimplemented!");

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarAccess(VarAccessExpression* node) {
  int offset = LookupVarAddress(node->name_);
  size_t size = current_frame_->LookupSize(node->name_).value();

  for (size_t i = 0; i < size; i++) {
    EmitMemFetch(offset + i);
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::codegen
