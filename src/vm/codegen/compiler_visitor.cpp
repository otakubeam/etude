#include <vm/codegen/compiler.hpp>
#include <vm/rt/native_table.hpp>

namespace vm::codegen {

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarDecl(VarDeclStatement* node) {
  // Generate code to place value on stack

  // TODO: set a flag that I am building this value

  node->value_->Accept(this);

  // Infrom FrameTranslator about this location

  auto name = node->GetVarName();

  current_frame_->AddLocal(name, current_frame_->GetNextSize());
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAssignment(AssignmentStatement* node) {
  node->value_->Accept(this);
  GenAddress(node->target_, true);

  auto size = GetTypeSize(node->target_);
  TranslateInstruction({
      .type = InstrType::STORE,
      .arg = size,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFunDecl(FunDeclStatement* node) {
  current_frame_ = new FrameTranslator{node, structs_};

  node->block_->Accept(this);

  TranslateInstruction({.type = InstrType::RET_FN});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFnCall(FnCallExpression* node) {
  FMT_ASSERT(false, "Unimplemented!");
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

  TranslateInstruction(FatInstr{
      .type = InstrType::RET_FN,
      // .debug_info = {.location = current_debug_location.top()},
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
  GenAddress(node->operand_, false);

  auto size = GetTypeSize(node->operand_);
  TranslateInstruction({
      .type = InstrType::LOAD,
      .arg = size,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAddressof(AddressofExpression* node) {
  // Vector instrs contains the recipe for calculating the address
  auto instrs = GenAddress(node->operand_, false);

  for (auto instr : *instrs) {
    TranslateInstruction(instr);
  }

  instrs->clear();
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitComparison(ComparisonExpression* node) {
  // node->left_->Accept(this);
  // node->right_->Accept(this);
  //
  // switch (node->operator_.type) {
  //   case lex::TokenType::EQUALS:
  //     chunk_.instructions.push_back({vm::FatInstr{
  //         .type = vm::InstrType::CMP_EQ,
  //     }});
  //     break;
  //
  //   case lex::TokenType::LT:
  //     chunk_.instructions.push_back({vm::FatInstr{
  //         .type = vm::InstrType::CMP_LESS,
  //     }});
  //     break;
  //
  //   default:
  //     FMT_ASSERT(false, "Unreachable!");
  // }
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
  // node->condition_->Accept(this);
  //
  // auto jump_to_false_ip = translator_.Count();
  //
  // chunk_.instructions.push_back(vm::FatInstr{
  //     .type = InstrType::JUMP_IF_FALSE,
  // });
  //
  // // True branch
  //
  // node->true_branch_->Accept(this);
  //
  // auto jump_to_end_ip = chunk_.instructions.size();
  //
  // chunk_.instructions.push_back(vm::FatInstr{
  //     .type = InstrType::JUMP,
  // });
  //
  // // False branch
  //
  // auto false_ip_start = chunk_.instructions.size();
  //
  // node->false_branch_->Accept(this);
  //
  // auto false_ip_end = chunk_.instructions.size();
  //
  // // Backpatch JUMP_IF_FALSE
  // chunk_.instructions.at(jump_to_false_ip) = vm::FatInstr{
  //     .type = InstrType::JUMP_IF_FALSE,
  //     .arg2 = (uint8_t)(false_ip_start >> 8),
  //     .arg3 = (uint8_t)(false_ip_start),
  // };
  //
  // // Backpatch JUMP
  // chunk_.instructions.at(jump_to_end_ip) = vm::FatInstr{
  //     .type = InstrType::JUMP,
  //     .arg2 = (uint8_t)(false_ip_end >> 8),
  //     .arg3 = (uint8_t)(false_ip_end),
  // };
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

  //
  // XXX: this doesn't work when I construct a structure
  // and then create some different varibale
  //
  // f(Str:{1,2,3});
  // var t = 123;         <<<-------- t will be the size of Str
  //
  current_frame_->SetNextPushSize(str_size);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFieldAccess(FieldAccessExpression* node) {
  // XXX: This is wrong, no?
  node->struct_expression_->Accept(this);

  auto offset = GetFieldOffset(node);

  // Here we compile the address right into the instruction

  // str.a.b = 5;

  // str.a.b;

  // Everything is known at compile time

  if (node->IsDirect()) {
    node->address_ = node->struct_expression_->GetAddress() + offset;
    EmitMemFetch(node->address_);
    return;
  }

  // Here we keep the address on stack and only add the offset

  // (*str.a).b = 5;

  // `.b` is indirect because deref is indirect
  // so we take this path and do not emit the `LOAD`

  // (*str.a).b;

  // here we emit the load

  GenAddress(node, false);
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
