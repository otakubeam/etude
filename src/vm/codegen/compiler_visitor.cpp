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
      .dbg_instr = {debug::DebuggerInstr{.var_name = node->GetVarName()}}};

  if (GetTypeSize(node->value_) > 1) {
    debug_info.dbg_instr->type_name =
        dynamic_cast<types::StructType*>(node->value_->GetType())->GetName();
  }

  translator_->LastDie() = std::move(debug_info);

  // Infrom FrameTranslator about this location

  auto name = node->GetVarName();
  auto size = GetTypeSize(node->value_);
  current_frame_->AddLocal(name, size);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAssignment(AssignmentStatement* node) {
  node->value_->Accept(this);
  auto instrs = GenAddress(node->target_, true);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();

  TranslateInstruction({
      .type = InstrType::STORE,
      .arg = GetTypeSize(node->target_),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFunDecl(FunDeclStatement* node) {
  //
  // Craft initial stack layout (parameters, registers)
  //
  current_frame_ = new FrameTranslator{node, structs_};

  node->block_->Accept(this);

  TranslateInstruction({.type = InstrType::RET_FN});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFnCall(FnCallExpression* node) {
  //
  // Place arguments in reverse order
  //
  for (int i = node->arguments_.size() - 1; i >= 0; i -= 1) {
    node->arguments_[i]->Accept(this);
  }

  if (node->GetFunctionName() == "isNull") {
    TranslateInstruction({
        .type = vm::InstrType::NATIVE_CALL,
        .arg = 0,
    });
    return;
  }

  // Branch direct / indirect

  if (auto mb_offset = current_frame_->LookupOffset(node->GetFunctionName())) {
    EmitMemFetch(*mb_offset);
    TranslateInstruction({
        .type = vm::InstrType::INDIRECT_CALL,
    });
  } else {
    TranslateInstruction({
        .type = vm::InstrType::CALL_FN,
        .fn_name = node->GetFunctionName(),
    });
  }

  // Don't forget to clean up the stack

  TranslateInstruction({
      .type = vm::InstrType::FIN_CALL,
      .arg = (uint8_t)node->arguments_.size(),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitStructDecl(StructDeclStatement* node) {
  structs_.Declare(node->GetStructName(),
                   new detail::StructSymbol{node, structs_});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitReturn(ReturnStatement* node) {
  // current_debug_location.push(node.GetLocation());
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

  TranslateInstruction(FatInstr{
      .type = InstrType::POP_STACK,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitDeref(DereferenceExpression* node) {
  // This is a path for rvalue conversion
  // Those who want lvalue (only '=`?) should just call GenAddress directly
  auto instrs = GenAddress(node, true);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();

  auto size = GetTypeSize(node->operand_);
  TranslateInstruction({
      .type = InstrType::LOAD,
      .arg = size,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAddressof(AddressofExpression* node) {
  // Vector instrs contains the recipe for calculating the address
  // TODO: assert that operand is not a deref expression
  auto instrs = GenAddress(node->operand_, true);

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

  switch (node->operator_.type) {
    case lex::TokenType::PLUS:
      type = InstrType::ADD;
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

void Compiler::VisitStructConstruction(StructConstructionExpression* node) {
  auto fetch = structs_.Get(node->GetStructName());
  auto str_size = fetch.value()->Size();

  // Generate code for placing all initializers on stack
  for (auto v : node->values_) {
    v->Accept(this);
  }

  current_frame_->SetNextPushSize(str_size);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFieldAccess(FieldAccessExpression* node) {
  auto instrs = GenAddress(node, false);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  TranslateInstruction({
      .type = InstrType::LOAD,
      .arg = GetTypeSize(node),
  });

  instrs->clear();
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

  current_frame_->SetNextPushSize(1);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarAccess(VarAccessExpression* node) {
  auto name = node->GetName();
  int offset = LookupVarAddress(name);
  size_t size = current_frame_->LookupSize(name).value();

  // TODO: maybe remove entirely
  node->address_ = offset;

  for (size_t i = 0; i < size; i++) {
    EmitMemFetch(offset + i);
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::codegen
