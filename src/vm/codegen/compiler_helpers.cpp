#include <vm/codegen/compiler.hpp>

namespace vm::codegen {

Compiler::Compiler() = default;
Compiler::~Compiler() = default;

////////////////////////////////////////////////////////////////////

ExecutableChunk Compiler::Compile(TreeNode* node) {
  node->Accept(this);
  return chunk_;
}

////////////////////////////////////////////////////////////////////

std::vector<ExecutableChunk>* Compiler::CompileScript(TreeNode* node) {
  auto result = new std::vector<ExecutableChunk>;

  Compiler c;
  c.compiled_chunks_ = result;
  c.current_frame_ = new FrameTranslator{};

  node->Accept(&c);

  result->push_back(c.chunk_);

  return result;
}

////////////////////////////////////////////////////////////////////

int Compiler::LookupVarAddress(std::string name) {
  auto mb_offset = current_frame_->Lookup(name);
  return mb_offset.value();
}

////////////////////////////////////////////////////////////////////

// Belongs to ExecutableChunk, move it!
void Compiler::AddIntegerConsnant(int value) {
  uint8_t const_no = chunk_.attached_vals.size();

  chunk_.attached_vals.push_back(rt::PrimitiveValue{
      .tag = rt::ValueTag::Int,
      .as_int = value,
  });

  chunk_.instructions.push_back(vm::Instr{
      .type = InstrType::PUSH_STACK,
      .arg1 = const_no,
  });
}

////////////////////////////////////////////////////////////////////

void Compiler::MabyeEmitMemFetch(int offset) {
  if (emit_mem_fetch_ == false) {
    return;
  }

  if (offset > 0) {
    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::GET_LOCAL,
        .arg1 = (uint8_t)offset,
    });
  } else {
    chunk_.instructions.push_back(vm::Instr{
        .type = vm::InstrType::GET_ARG,
        .arg1 = (uint8_t)(-offset),
    });
  }
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::codegen
