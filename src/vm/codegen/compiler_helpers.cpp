#include <vm/codegen/compiler.hpp>

namespace vm::codegen {

////////////////////////////////////////////////////////////////////

Compiler::Compiler() {
  current_frame_ = new FrameTranslator{};
};

Compiler::~Compiler() = default;

////////////////////////////////////////////////////////////////////

auto Compiler::Compile(TreeNode* node) -> ElfFile {
  if (auto fn = dynamic_cast<FunDeclStatement*>(node)) {
    translator_.emplace(InstrTranslator(fn->GetFunctionName()));
    fn->Accept(this);
    return std::move(translator_.value()).Finalize();
  } else if (auto _struct = dynamic_cast<StructDeclStatement*>(node)) {
    _struct->Accept(this);
  }

  return ElfFile{{}, {}, {}, {}};
}

////////////////////////////////////////////////////////////////////

int Compiler::LookupVarAddress(std::string name) {
  auto mb_offset = current_frame_->LookupOffset(name);
  return mb_offset.value();
}

////////////////////////////////////////////////////////////////////

void Compiler::EmitMemFetch(int16_t offset) {
  TranslateInstruction({
      .type = vm::InstrType::GET_AT_FP,
      .offset = offset,
  });
}

}  // namespace vm::codegen
