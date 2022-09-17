#include <vm/codegen/compiler.hpp>

namespace vm::codegen {

////////////////////////////////////////////////////////////////////

void Compiler::VisitStatement(Statement* /* node */) {
  FMT_ASSERT(false, "Visiting bare statement");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarDecl(VarDeclStatement* node) {
  // Generate code to place value on stack
  node->value_->Accept(this);

  // Infrom FrameTranslator about this location
  auto name = node->lvalue_->name_;
  current_frame_->AddLocal(name.GetName(), current_frame_->GetNextSize());
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAssignment(AssignmentStatement* node) {
  node->value_->Accept(this);

  {
    emit_mem_fetch_ = false;
    node->target_->Accept(this);
    emit_mem_fetch_ = true;
  }

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::STORE_STACK,
      .addr = (int16_t)node->target_->GetAddress(),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFunDecl(FunDeclStatement* node) {
  FrameTranslator builder{node, structs_};

  Compiler chunk_compiler;

  // TODO: avoid copying
  chunk_compiler.structs_ = structs_;
  chunk_compiler.current_frame_ = &builder;
  // For the case of nested fn declarations
  chunk_compiler.compiled_chunks_ = compiled_chunks_;

  auto chunk = chunk_compiler.Compile(node->block_);

  chunk.instructions.push_back(vm::Instr{
      .type = InstrType::RET_FN,
  });

  int chunk_no = compiled_chunks_->size();
  uint8_t const_no = chunk_.attached_vals.size();

  chunk_.attached_vals.push_back(rt::PrimitiveValue{
      .tag = rt::ValueTag::Int,
      .as_int = chunk_no,
  });

  compiled_chunks_->push_back(chunk);

  // This allows for lookup of this symbol later at the callsite
  current_frame_->AddLocal(node->name_.GetName());

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::PUSH_STACK,
      .arg1 = const_no,  // push the constant of the compiled chunk
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFnCall(FnCallExpression* node) {
  // Place in reverse order
  for (int i = node->arguments_.size() - 1; i >= 0; i -= 1) {
    node->arguments_[i]->Accept(this);
  }

  if (node->is_native_call_) {
    AddIntegerConsnant(node->arguments_.size());
    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::NATIVE_CALL,
        .arg1 = 0,  // print
    });
    return;
  }

  auto mb_offset = current_frame_->Lookup(node->fn_name_.GetName());

  int offset = mb_offset.value();

  // TODO: branch direct / indirect

  MabyeEmitMemFetch(offset);

  chunk_.instructions.push_back(vm::Instr{
      .type = vm::InstrType::INDIRECT_CALL,
  });

  chunk_.instructions.push_back(vm::Instr{
      .type = vm::InstrType::FIN_CALL,
      .arg1 = (uint8_t)node->arguments_.size(),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitStructDecl(StructDeclStatement* node) {
  structs_.Declare(node->name_.GetName(),
                   new detail::StructSymbol{node, structs_});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitReturn(ReturnStatement*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitYield(YieldStatement*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitExprStatement(ExprStatement* node) {
  node->expr_->Accept(this);

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::POP_STACK,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitExpression(Expression*) {
  FMT_ASSERT(false, "Unreachable!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitDeref(DereferenceExpression*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAddressof(AddressofExpression*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitComparison(ComparisonExpression* node) {
  node->left_->Accept(this);
  node->right_->Accept(this);

  switch (node->operator_.type) {
    case lex::TokenType::EQUALS:
      chunk_.instructions.push_back({vm::Instr{
          .type = vm::InstrType::CMP_EQ,
      }});
      break;

    case lex::TokenType::LT:
      // a < b <=> b - a > 0
      FMT_ASSERT(false, "Unimplemented!");
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitBinary(BinaryExpression* node) {
  node->left_->Accept(this);
  node->right_->Accept(this);

  switch (node->operator_.type) {
    case lex::TokenType::PLUS:
      chunk_.instructions.push_back(vm::Instr{
          .type = InstrType::ADD,
      });
      break;

    case lex::TokenType::MINUS:
      chunk_.instructions.push_back(vm::Instr{
          .type = InstrType::SUBTRACT,
      });
      break;

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

void Compiler::VisitUnary(UnaryExpression*) {
  FMT_ASSERT(false, "Unimplemented!");
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitIf(IfExpression* node) {
  /////////

  node->condition_->Accept(this);

  auto jump_to_false_ip = chunk_.instructions.size();

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::JUMP_IF_FALSE,
  });

  /////////

  node->true_branch_->Accept(this);

  auto jump_to_end_ip = chunk_.instructions.size();

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::JUMP,
  });

  /////////

  auto false_ip_start = chunk_.instructions.size();

  node->false_branch_->Accept(this);

  auto false_ip_end = chunk_.instructions.size();

  // Backpatch JUMP_IF_FALSE
  chunk_.instructions.at(jump_to_false_ip) = vm::Instr{
      .type = InstrType::JUMP_IF_FALSE,
      .arg2 = (uint8_t)(false_ip_start >> 8),
      .arg3 = (uint8_t)(false_ip_start),
  };

  // Backpatch JUMP
  chunk_.instructions.at(jump_to_end_ip) = vm::Instr{
      .type = InstrType::JUMP,
      .arg2 = (uint8_t)(false_ip_end >> 8),
      .arg3 = (uint8_t)(false_ip_end),
  };
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitBlock(BlockExpression* node) {
  for (auto& st : node->stmts_) {
    st->Accept(this);
  }

  if (node->final_) {
    node->final_->Accept(this);
    return;
  }

  AddIntegerConsnant(999 /* mark */);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitStructConstruction(StructConstructionExpression* node) {
  auto fetch = structs_.Get(node->struct_name_.GetName());
  auto str_size = fetch.value()->Size();

  // Generate code for placing all initializers on stack
  for (auto v : node->values_) {
    v->Accept(this);
  }

  current_frame_->SetNextPushSize(str_size);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFieldAccess(FieldAccessExpression* node) {
  // Do this for the first time on the top level
  // (emit mem fetch only on the top level)
  bool top_level = std::exchange(emit_mem_fetch_, false);

  node->struct_expression_->Accept(this);

  // Cancel the effect on the top level
  if (top_level) {
    emit_mem_fetch_ = true;
  }

  // This succeeds because of typechecking
  auto t =
      dynamic_cast<types::StructType*>(node->struct_expression_->GetType());

  auto struct_type_name = t->GetName();

  // Symbol encapsulate knowledge about the size of primitive fields
  // which might be differenct from implementation to implementation
  auto struct_symbol = structs_.Get(struct_type_name).value();

  auto offset = struct_symbol->SizeBefore(node->field_name_.GetName());

  node->address_ = node->struct_expression_->GetAddress() + offset;

  MabyeEmitMemFetch(node->address_);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitLiteral(LiteralExpression* lit) {
  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::PUSH_STACK,
      .arg1 = (uint8_t)chunk_.attached_vals.size(),
  });

  switch (lit->token_.type) {
    case lex::TokenType::NUMBER: {
      chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
          .tag = rt::ValueTag::Int,
          .as_int = std::get<int>(lit->token_.sem_info),
      });

      break;
    }

    case lex::TokenType::STRING:
      FMT_ASSERT(false, "Unimplemented!");

    case lex::TokenType::TRUE: {
      chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
          .tag = rt::ValueTag::Bool,
          .as_bool = true,
      });

      break;
    }

    case lex::TokenType::FALSE: {
      chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
          .tag = rt::ValueTag::Bool,
          .as_bool = false,
      });

      break;
    }

    default:
      FMT_ASSERT(false, "Unreachable!");
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarAccess(VarAccessExpression* node) {
  auto name = node->name_.GetName();
  int offset = LookupVarAddress(name);

  node->address_ = offset;

  if (!node->GetType()->IsStruct()) {
    MabyeEmitMemFetch(offset);
    return;
  }

  // Deep struct copy

  // Guaranteed to be non-zero
  auto str = dynamic_cast<types::StructType*>(node->GetType());

  auto type_name = str->GetName();
  auto type_size = structs_.Get(type_name).value()->Size();

  for (size_t i = 0; i < type_size; i++) {
    MabyeEmitMemFetch(offset + i);
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::codegen
