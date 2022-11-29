#include <vm/codegen/compiler.hpp>

namespace vm::codegen {

////////////////////////////////////////////////////////////////////

Compiler::Compiler() {
  current_frame_ = new FrameTranslator{};
};

Compiler::~Compiler() = default;

////////////////////////////////////////////////////////////////////

auto Compiler::Compile(TreeNode* node) -> ElfFile {
  if (auto fn = node->as<FunDeclStatement>()) {
    std::string mangled = std::string(fn->GetName());
    mangled += types::Mangle(*fn->type_);

    translator_.emplace(InstrTranslator(std::move(mangled)));

    fn->Accept(this);
    return std::move(translator_.value()).Finalize();
  }

  FMT_ASSERT(false, "Unreachable");
}

////////////////////////////////////////////////////////////////////

int Compiler::LookupVarAddress(std::string_view name) {
  auto mb_offset = current_frame_->LookupOffset(name);
  return mb_offset.value();
}

////////////////////////////////////////////////////////////////////

void Compiler::EmitMemFetch(int16_t offset) {
  TranslateInstruction({
      .type = InstrType::GET_AT_FP,
      .offset = offset,
  });
}

////////////////////////////////////////////////////////////////////

auto Compiler::GenAddress(Expression* expr) -> std::vector<FatInstr>* {
  static std::vector<FatInstr> generation;

  if (auto lit = expr->as<VarAccessExpression>()) {
    int16_t offset = LookupVarAddress(lit->name_);
    generation.push_back(FatInstr{.type = InstrType::PUSH_FP});
    generation.push_back({.type = InstrType::ADD_ADDR, .offset = offset});
    return &generation;
  }

  if (auto deref = expr->as<DereferenceExpression>()) {
    deref->operand_->Accept(this);
    return &generation;
  }

  if (auto field_access = expr->as<FieldAccessExpression>()) {
    GenAddress(field_access->struct_expression_);
    generation.push_back(FatInstr{
        .type = InstrType::ADD_ADDR,
        .offset = GetFieldOffset(field_access),
    });

    return &generation;
  }

  std::abort();
}

////////////////////////////////////////////////////////////////////

int16_t Compiler::GetFieldOffset(FieldAccessExpression* fa) {
  return measure_.MeasureFieldOffset(fa->struct_expression_->GetType(),
                                     fa->field_name_);
}

////////////////////////////////////////////////////////////////////

int16_t Compiler::GetFieldOffset(types::Type* t, std::string_view field) {
  return measure_.MeasureFieldOffset(t, field);
}

////////////////////////////////////////////////////////////////////

uint8_t Compiler::GetValueSize(Expression* exp) {
  return GetTypeSize(exp->GetType());
}
////////////////////////////////////////////////////////////////////

uint8_t Compiler::GetTypeSize(types::Type* t) {
  return measure_.MeasureSize(t);
}

////////////////////////////////////////////////////////////////////

void Compiler::TranslateInstruction(FatInstr instr) {
  translator_->TranslateInstruction(instr);
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::codegen
