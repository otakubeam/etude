#pragma once

#include <types/type.hpp>
#include <types/trait.hpp>

#include <ast/scope/context.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <ast/declarations.hpp>

#include <queue>

namespace types::check {

class TemplateInstantiator : public ReturnVisitor<TreeNode*> {
 public:
  bool TryFindInstantiation(FnCallExpression* i) {
    auto range = mono_items_.equal_range(i->GetFunctionName());

    for (auto it = range.first; it != range.second; ++it) {
      if (TypesEquivalent(it->second->type_, i->callable_type_)) {
        return true;
      }
    }
    return false;
  }

  void StartUp(FunDeclStatement* main) {
    auto main_fn = Eval(main)->as<FunDeclStatement>();
    mono_items_.insert({main_fn->name_, main_fn});
  }

  void ProcessQueueItem(FnCallExpression* i) {
    // 1) Check not already instantiated

    if (TryFindInstantiation(i)) {
      return;
    }

    fmt::print(stderr, "[!] Symbol\n");
    auto symbol = i->layer_->RetrieveSymbol(i->fn_name_);

    fmt::print(stderr, "[!] Symbol\n");

    // 2) Enter context

    auto poly = symbol->GetType();
    fmt::print(stderr, "[!] Poly {}\n", FormatType(*poly));
    auto mono = i->callable_type_;
    fmt::print(stderr, "[!] Mono {}\n", FormatType(*mono));

    poly_to_mono_.clear();
    BuildPolyToMono(poly, mono, poly_to_mono_);

    // 3) Find definition

    auto definition = symbol->as_fn_sym.def;

    // 4) Evaluate

    auto mono_fun = Eval(definition)->as<FunDeclStatement>();

    // 5) Save result
    if (mono_fun->body_) {
      mono_items_.insert({mono_fun->name_, mono_fun});
    }
  }

  void ProcessQueue() {
    while (instantiation_quque_.size()) {
      auto i = instantiation_quque_.front();
      fmt::print(stderr, "[!] Processing item\n");
      ProcessQueueItem(i);
      fmt::print(stderr, "[!] Processed item\n");
      instantiation_quque_.pop_front();
    }
  }

  TemplateInstantiator(Declaration* main) {
    StartUp(main->as<FunDeclStatement>());

    fmt::print(stderr, "Finished processing main\n");

    ProcessQueue();
  }

  auto Flush()
      -> std::pair<std::vector<FunDeclStatement*>, std::vector<Type*>> {
    std::vector<FunDeclStatement*> result;

    for (auto& mono : mono_items_) {
      fmt::print(stderr, "name: {} type: {}\n", mono.second->GetName(),
                 FormatType(*mono.second->type_));

      result.push_back(mono.second);
    }
    return {result, types_to_gen_};
  }

  void BuildPolyToMono(Type* poly, Type* mono,
                       std::unordered_map<Type*, Type*>& poly_to_mono) {
    poly = FindLeader(poly);
    mono = FindLeader(mono);

    switch (poly->tag) {
      case TypeTag::TY_PTR:
        FMT_ASSERT(mono->tag == TypeTag::TY_PTR, "Mismatch");
        BuildPolyToMono(poly->as_ptr.underlying, mono->as_ptr.underlying,
                        poly_to_mono);
        break;

      case TypeTag::TY_STRUCT: {
        auto& a_mem = poly->as_struct.first;
        auto& b_mem = mono->as_struct.first;

        if (a_mem.size() != b_mem.size()) {
          throw std::runtime_error{"Inference error: struct size mismatch"};
        }

        for (size_t i = 0; i < a_mem.size(); i++) {
          BuildPolyToMono(a_mem[i].ty, b_mem[i].ty, poly_to_mono);
        }

        break;
      }

      case TypeTag::TY_FUN: {
        auto& pack = poly->as_fun.param_pack;
        auto& pack2 = mono->as_fun.param_pack;

        if (pack.size() != pack2.size()) {
          throw std::runtime_error{"Function unification size mismatch"};
        }

        for (size_t i = 0; i < pack.size(); i++) {
          BuildPolyToMono(pack[i], pack2[i], poly_to_mono);
        }

        BuildPolyToMono(poly->as_fun.result_type, mono->as_fun.result_type,
                        poly_to_mono);
        break;
      }

      case TypeTag::TY_APP: {
        if (poly->as_tyapp.name.GetName() != mono->as_tyapp.name) {
          std::abort();
        }

        auto& a_pack = poly->as_tyapp.param_pack;
        auto& b_pack = mono->as_tyapp.param_pack;

        for (size_t i = 0; i < a_pack.size(); i++) {
          BuildPolyToMono(a_pack[i], b_pack[i], poly_to_mono);
        }

        break;
      }

        // G13 -> Int
      case TypeTag::TY_PARAMETER:
        // This function exists for this callback
        poly_to_mono_.insert({poly, mono});
        break;

      case TypeTag::TY_CONS:
      case TypeTag::TY_KIND:
      case TypeTag::TY_VARIABLE:
      case TypeTag::TY_UNION:
        FMT_ASSERT(false, "Unreachable");

      default:
        break;
    }
  }

  void VisitYield(YieldStatement* node);
  void VisitReturn(ReturnStatement* node);
  void VisitAssignment(AssignmentStatement* node);
  void VisitExprStatement(ExprStatement* node);

  void VisitTypeDecl(TypeDeclStatement* node);
  void VisitVarDecl(VarDeclStatement* node);
  void VisitFunDecl(FunDeclStatement* node);
  void VisitTraitDecl(TraitDeclaration* node);

  void VisitComparison(ComparisonExpression* node);
  void VisitBinary(BinaryExpression* node);
  void VisitUnary(UnaryExpression* node);
  void VisitDeref(DereferenceExpression* node);
  void VisitAddressof(AddressofExpression* node);
  void VisitIf(IfExpression* node);
  void VisitNew(NewExpression* node);
  void VisitBlock(BlockExpression* node);
  void VisitFnCall(FnCallExpression* node);
  void VisitIntrinsic(IntrinsicCall* node);
  void VisitFieldAccess(FieldAccessExpression* node);
  void VisitTypecast(TypecastExpression* node);
  void VisitLiteral(LiteralExpression* node);
  void VisitVarAccess(VarAccessExpression* node);
  void VisitCompoundInitalizer(CompoundInitializerExpr* node);

 private:
  void MaybeSaveForIL(Type* ty) {
    if (ty->tag != TypeTag::TY_APP) {
      return;
    }

    auto storage = TypeStorage(ty);

    if (storage->tag <= TypeTag::TY_PTR) {
      return;
    }

    types_to_gen_.push_back(ty);
  }

 private:
  std::unordered_map<Type*, Type*> poly_to_mono_;

  std::deque<FnCallExpression*> instantiation_quque_;

  std::vector<Type*> types_to_gen_;

  // How do I prevent myself from instantiating something twice or more?
  // A: place instantiated in map: name: string_view -> [](type, fun)

  std::unordered_multimap<std::string_view, FunDeclStatement*> mono_items_;
};

}  // namespace types::check
