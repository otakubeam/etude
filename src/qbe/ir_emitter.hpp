#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>
#include <qbe/measure.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <ast/declarations.hpp>
#include <ast/patterns.hpp>

#include <unordered_map>
#include <utility>

namespace qbe {

class GenMatch;
class GenAddr;
class GenAt;

class IrEmitter : public ReturnVisitor<Value> {
 public:
  friend class GenMatch;
  friend class GenAddr;
  friend class GenAt;

  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitReturn(ReturnStatement* node) override;
  virtual void VisitYield(YieldStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;
  virtual void VisitTypeDecl(TypeDeclStatement* node) override;
  virtual void VisitTraitDecl(TraitDeclaration* node) override;

  virtual void VisitBindingPat(BindingPattern* node) override;
  virtual void VisitLiteralPat(LiteralPattern* node) override;
  virtual void VisitVariantPat(VariantPattern* node) override;

  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
  virtual void VisitMatch(MatchExpression* node) override;
  virtual void VisitNew(NewExpression* node) override;
  virtual void VisitBlock(BlockExpression* node) override;
  virtual void VisitComparison(ComparisonExpression* node) override;
  virtual void VisitBinary(BinaryExpression* node) override;
  virtual void VisitUnary(UnaryExpression*) override;
  virtual void VisitFnCall(FnCallExpression* node) override;
  virtual void VisitIntrinsic(IntrinsicCall* node) override;
  virtual void VisitFieldAccess(FieldAccessExpression* node) override;
  virtual void VisitVarAccess(VarAccessExpression* node) override;
  virtual void VisitLiteral(LiteralExpression* node) override;
  virtual void VisitTypecast(TypecastExpression* node) override;
  virtual void VisitCompoundInitalizer(CompoundInitializerExpr* node) override;

 private:
  auto SizeAlign(types::Type* ty) {
    return std::pair(GetTypeSize(ty), measure_.MeasureAlignment(ty));
  }

  auto SizeAlign(Expression* node) {
    auto ty = node->GetType();
    return SizeAlign(ty);
  }

 public:
  void EmitType(types::Type* ty) {
    if (ty->tag != types::TypeTag::TY_APP) {
      return;
    }

    auto storage = types::TypeStorage(ty);

    if (storage->tag <= types::TypeTag::TY_PTR) {
      return;
    }

    switch (storage->tag) {
      case types::TypeTag::TY_STRUCT: {
        auto& members = storage->as_struct.first;

        for (auto& mem : members) {
          EmitType(mem.ty);
        }

        fmt::print("type :{} = {{ ", Mangle(*ty));

        for (auto& mem : members) {
          fmt::print("{} {}, ", ToQbeType(mem.ty), 1);
        }

        fmt::print("}}\n");
        break;
      }

      case types::TypeTag::TY_SUM: {
        auto& members = storage->as_sum.first;

        for (auto& mem : members) {
          EmitType(mem.ty);
        }

        fmt::print("type :{} = {{ w 1, ", Mangle(*ty));

        auto size = measure_.MeasureSum(storage);
        fmt::print(" w {} ", size / 4 - 1);

        fmt::print("}}\n");
        break;
      }

      default:
        std::abort();
    }
  }

  void EmitTypes(std::vector<types::Type*> types) {
    for (auto ty : types) {
      EmitType(ty);
    }
  }

  ~IrEmitter() {
    // data $strdata.0 = { b "hello", b 0 }

    for (size_t i = 0; i < string_literals_.size(); i++) {
      fmt::print("data $strdata.{} = {{ b \"{}\", b 0 }}", i,
                 string_literals_[i]);
      fmt::print("\n");
    }
  }

 private:
  char GetStoreSuf(size_t align) {
    switch (align) {
      case 1:
        return 'b';
      case 4:
        return 'w';
      case 8:
        return 'l';
      default:
        std::abort();
    }
  }

  std::string_view GetLoadSuf(size_t align) {
    switch (align) {
      case 1:
        return "ub";
      case 4:
        return "w";
      case 8:
        return "l";
      default:
        std::abort();
    }
  }

  std::string_view LoadResult(size_t align) {
    switch (align) {
      case 1:
      case 4:
        return "w";
      case 8:
        return "l";
      default:
        std::abort();
    }
  }

  void Copy(size_t align, size_t size, Value src, Value dst) {
    auto str_suf = GetStoreSuf(align);
    auto ld_suf = GetLoadSuf(align);
    auto ld_res = LoadResult(align);

    auto temp = GenTemporary();
    auto src_ptr = GenTemporary();
    auto dst_ptr = GenTemporary();

    fmt::print("  {} = l copy {}\n", src_ptr.Emit(), src.Emit());
    fmt::print("  {} = l copy {}\n", dst_ptr.Emit(), dst.Emit());

    for (size_t copied = 0; copied < size; copied += align) {
      fmt::print("# Copying \n");
      fmt::print("  {} = {} load{} {}\n", temp.Emit(), ld_res, ld_suf,
                 src_ptr.Emit());
      fmt::print("  store{} {}, {}  \n", str_suf, temp.Emit(), dst_ptr.Emit());

      fmt::print("  {} =l add {}, {}\n", src_ptr.Emit(), src_ptr.Emit(), align);
      fmt::print("  {} =l add {}, {}\n", dst_ptr.Emit(), dst_ptr.Emit(), align);
    }
  }

  uint8_t GetTypeSize(types::Type* t) {
    return measure_.MeasureSize(t);
  }

  void CallPrintf(IntrinsicCall* node) {
    auto fmt = Eval(node->arguments_[0]);

    std::deque<Value> values;

    for (auto& a : std::span(node->arguments_).subspan(1)) {
      values.push_back(Eval(a));
    }

    fmt::print("  call $printf (l {}, ..., ", fmt.Emit());

    for (auto& a : std::span(node->arguments_).subspan(1)) {
      auto value = std::move(values.front());
      fmt::print("{} {}, ", ToQbeType(a->GetType()), value.Emit());
      values.pop_front();
    }

    fmt::print(")\n");
  }

  void CheckAssertion(Expression* cond) {
    auto true_id = id_ += 1;
    auto false_id = id_ += 1;
    auto join_id = id_ += 1;

    auto condition = Eval(cond);

    fmt::print("#if-start\n");
    fmt::print("  jnz {}, @true.{}, @false.{}\n", condition.Emit(), true_id,
               false_id);

    fmt::print("@true.{}          \n", true_id);
    // Do nothing
    fmt::print("  jmp @join.{}    \n", join_id);

    fmt::print("@false.{}         \n", false_id);
    CallAbort(cond);

    fmt::print("@join.{}          \n", join_id);
  }

  void CallAbort(Expression* cond) {
    error_msg_storage_.push_back(
        fmt::format("Abort at {}\\n", cond->GetLocation().Format()));

    string_literals_.push_back(error_msg_storage_.back());

    fmt::print("  call $printf (l $strdata.{}, ..., ) \n",
               string_literals_.size() - 1);
    fmt::print("  call $abort ()  \n");
  }

  Value GenParam() {
    return {.tag = Value::PARAM, .id = id_ += 1};
  }

  Value GenTemporary() {
    return {.tag = Value::TEMPORARY, .id = id_ += 1};
  }

  Value GenConstInt(int value) {
    return {.tag = Value::CONST_INT, .value = value};
  }

  Value GenGlobal(std::string_view name) {
    return {.tag = Value::GLOBAL, .name = std::string{name}};
  }

  void GenAddress(Expression* what, Value out);

  void GenAtAddress(Expression* what, Value where);

 private:
  int id_ = 0;
  std::unordered_map<std::string_view, Value> named_values_;

  std::vector<std::string_view> string_literals_;
  std::vector<std::string> error_msg_storage_;

  detail::SizeMeasure measure_;
};

}  // namespace qbe
