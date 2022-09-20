#include <vm/codegen/compiler.hpp>
#include <vm/rt/native_table.hpp>

namespace vm::codegen {

////////////////////////////////////////////////////////////////////

void Compiler::VisitVarDecl(VarDeclStatement* node) {
  // Generate code to place value on stack
  node->value_->Accept(this);

  // Infrom FrameTranslator about this location
  auto name = node->GetVarName();
  current_frame_->AddLocal(name, current_frame_->GetNextSize());
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAssignment(AssignmentStatement* node) {
  node->value_->Accept(this);

  {
    emit_mem_fetch_ = false;
    node->target_->Accept(this);
    emit_mem_fetch_ = true;
  }

  if (node->target_->IsDirect()) {
    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::STORE_STACK,
        .addr = (int16_t)node->target_->GetAddress(),
    });
  } else {
    // The address is on top of the stack
    // The value is just below it
    chunk_.instructions.push_back(vm::Instr{
        .type = InstrType::STORE,
    });
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFunDecl(FunDeclStatement* node) {
  // Make space for our own chunk

  auto saved_size = compiled_chunks_->size();

  compiled_chunks_->push_back(ExecutableChunk{});

  functions_.Declare(node->GetFunctionName(), saved_size);

  // Build frame

  FrameTranslator builder{node, structs_};

  Compiler chunk_compiler;

  chunk_compiler.structs_ = structs_;      // TODO: avoid copying
  chunk_compiler.functions_ = functions_;  // TODO: avoid copying
  chunk_compiler.current_frame_ = &builder;
  // For the case of nested fn declarations (TODO: unnecessary?)
  chunk_compiler.compiled_chunks_ = compiled_chunks_;

  auto chunk = chunk_compiler.Compile(node->block_);

  chunk.instructions.push_back(vm::Instr{
      .type = InstrType::RET_FN,
  });

  // Rewrite our chunk now that we have the executable
  compiled_chunks_->at(saved_size) = chunk;

  // This allows for lookup of this symbol later at the callsite
  current_frame_->AddLocal(node->GetFunctionName());

  AddIntegerConsnant(saved_size);
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitFnCall(FnCallExpression* node) {
  // Place in reverse order

  for (int i = node->arguments_.size() - 1; i >= 0; i -= 1) {
    node->arguments_[i]->Accept(this);
  }

  if (node->is_native_call_) {
    auto name = node->GetFunctionName();
    auto offset = name == "print" ? 0 : 2;

    AddIntegerConsnant(node->arguments_.size());

    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::NATIVE_CALL,
        .arg1 = (uint8_t)offset,
    });

    return;
  }

  // Branch direct / indirect

  if (auto mb_offset = current_frame_->Lookup(node->GetFunctionName())) {
    int offset = mb_offset.value();
    MabyeEmitMemFetch(offset);

    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::INDIRECT_CALL,
    });
  } else {
    auto chunk_no = functions_.Get(node->GetFunctionName()).value().GetAddr();

    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::CALL_FN,
        .arg1 = (uint8_t)chunk_no,
    });
  }

  // Don't forget to clean up the stack

  chunk_.instructions.push_back(vm::Instr{
      .type = vm::InstrType::FIN_CALL,
      .arg1 = (uint8_t)node->arguments_.size(),
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitStructDecl(StructDeclStatement* node) {
  structs_.Declare(node->GetStructName(),
                   new detail::StructSymbol{node, structs_});
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitReturn(ReturnStatement* node) {
  node->return_value_->Accept(this);

  chunk_.instructions.push_back(vm::Instr{
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

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::POP_STACK,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitDeref(DereferenceExpression* node) {
  auto prev_state = emit_mem_fetch_;

  // Put operand's address on stack
  emit_mem_fetch_ = true;

  node->operand_->Accept(this);

  emit_mem_fetch_ = prev_state;

  // node->IsDirect() is false

  if (emit_mem_fetch_) {
    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::LOAD,
    });
  }
}

////////////////////////////////////////////////////////////////////

void Compiler::VisitAddressof(AddressofExpression* node) {
  {
    emit_mem_fetch_ = false;
    node->operand_->Accept(this);
    emit_mem_fetch_ = true;
  }

  // Now I have the address of operand_!

  if (node->operand_->IsDirect()) {
    auto addr = node->operand_->GetAddress();
    AddIntegerConsnant(addr);
  } else {
    // No-op
  }
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
      chunk_.instructions.push_back({vm::Instr{
          .type = vm::InstrType::CMP_LESS,
      }});
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
  node->condition_->Accept(this);

  auto jump_to_false_ip = chunk_.instructions.size();

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::JUMP_IF_FALSE,
  });

  // True branch

  node->true_branch_->Accept(this);

  auto jump_to_end_ip = chunk_.instructions.size();

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::JUMP,
  });

  // False branch

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
  for (auto& statement : node->stmts_) {
    statement->Accept(this);
  }

  if (node->final_) {
    node->final_->Accept(this);
    return;
  }

  AddIntegerConsnant(999 /* mark */);
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

  auto offset = struct_symbol->SizeBefore(node->GetFieldName());

  // Here we compile the address right into the instruction

  // str.a.b = 5;

  // str.a.b;

  // Everything is known at compile time

  if (node->IsDirect()) {
    node->address_ = node->struct_expression_->GetAddress() + offset;
    MabyeEmitMemFetch(node->address_);
    return;
  }

  // Here we keep the address on stack and only add the offset

  // (*str.a).b = 5;

  // `.b` is indirect because deref is indirect
  // so we take this path and do not emit the `LOAD`

  // (*str.a).b;

  // here we emit the load

  AddIntegerConsnant(offset);
  chunk_.instructions.push_back(Instr{.type = InstrType::ADD});

  if (emit_mem_fetch_) {
    chunk_.instructions.push_back(Instr{.type = InstrType::LOAD});
  }
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

    case lex::TokenType::UNIT:
      chunk_.attached_vals.push_back(vm::rt::PrimitiveValue{
          .tag = rt::ValueTag::Int,
          .as_int = 1234,
      });
      break;

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
  auto name = node->GetName();
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
