#pragma once

#include <qbe/qbe_value.hpp>
#include <qbe/qbe_types.hpp>
#include <qbe/measure.hpp>

#include <ast/visitors/template_visitor.hpp>

#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <unordered_map>
#include <utility>

namespace qbe {

class GenAddr;
class GenAt;

class IrEmitter : public ReturnVisitor<Value> {
 public:
  friend class GenAddr;
  friend class GenAt;

  virtual void VisitVarDecl(VarDeclStatement* node) override;
  virtual void VisitAssignment(AssignmentStatement* node) override;
  virtual void VisitFunDecl(FunDeclStatement* node) override;
  virtual void VisitTypeDecl(TypeDeclStatement* node) override;
  virtual void VisitReturn(ReturnStatement* node) override;
  virtual void VisitYield(YieldStatement* node) override;
  virtual void VisitExprStatement(ExprStatement* node) override;

  virtual void VisitDeref(DereferenceExpression* node) override;
  virtual void VisitAddressof(AddressofExpression* node) override;
  virtual void VisitIf(IfExpression* node) override;
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

  ~IrEmitter() {
    // data $strdata.0 = { b "hello", b 0 }

    for (size_t i = 0; i < string_literals_.size(); i++) {
      fmt::print("data $strdata.{} = {{ b \"{}\", b 0 }}", i,
                 string_literals_[i]);
      fmt::print("\n");
    }
  }

 private:
  void GenAt(Expression* what, Value where) {
    auto out = Eval(what);
    fmt::print("  store{} {}, {}\n",  //
               StoreSuf(what->GetType()), out.Emit(), where.Emit());
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
    CallAbort();

    fmt::print("@join.{}          \n", join_id);
  }

  void CallAbort() {
    fmt::print("  call $abort ()\n");
  }

  Value GenParam() {
    return {
        .tag = Value::PARAM,
        .id = id_ += 1,
    };
  }

  Value GenTemporary() {
    return {
        .tag = Value::TEMPORARY,
        .id = id_ += 1,
    };
  }

  Value GenConstInt(int value) {
    return {
        .tag = Value::CONST_INT,
        .value = value,
    };
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
  std::vector<types::Type*> types_;

  detail::SizeMeasure measure_;
};

}  // namespace qbe
