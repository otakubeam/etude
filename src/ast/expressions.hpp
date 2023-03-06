#pragma once

#include <ast/elaboration/intrinsics.hpp>

#include <ast/scope/context.hpp>
#include <ast/syntax_tree.hpp>

#include <types/type.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Pattern;

//////////////////////////////////////////////////////////////////////

class Expression : public TreeNode {
 public:
  virtual void Accept(Visitor* /* visitor */){};

  virtual types::Type* GetType() = 0;
};

//////////////////////////////////////////////////////////////////////

class ComparisonExpression : public Expression {
 public:
  ComparisonExpression(Expression* left, lex::Token op, Expression* right)
      : left_{left}, operator_(op), right_{right} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitComparison(this);
  }

  virtual types::Type* GetType() override {
    return &types::builtin_bool;
  };

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

  Expression* left_;
  lex::Token operator_;
  Expression* right_;
};

//////////////////////////////////////////////////////////////////////

class BinaryExpression : public Expression {
 public:
  BinaryExpression(Expression* left, lex::Token op, Expression* right)
      : left_{left}, operator_(op), right_{right} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitBinary(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Type is not set");
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

  Expression* left_;
  lex::Token operator_;
  Expression* right_;

  types::Type* type_ = nullptr;

  bool is_pointer_arithmetic_ = false;
};

//////////////////////////////////////////////////////////////////////

class UnaryExpression : public Expression {
 public:
  UnaryExpression(lex::Token op, Expression* operand)
      : operator_(op), operand_{operand} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitUnary(this);
  }

  virtual types::Type* GetType() override {
    return operand_->GetType();
  };

  virtual lex::Location GetLocation() override {
    return operator_.location;
  }

  lex::Token operator_;
  Expression* operand_;
};

//////////////////////////////////////////////////////////////////////

class DereferenceExpression : public Expression {
 public:
  DereferenceExpression(lex::Token star, Expression* operand)
      : star_{star}, operand_{operand} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitDeref(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Typechecker fault!");
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return star_.location;
  }

  lex::Token star_;

  // The pointer expression
  Expression* operand_;

  types::Type* type_ = nullptr;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class AddressofExpression : public Expression {
 public:
  AddressofExpression(lex::Token ampersand, Expression* operand)
      : ampersand_{ampersand}, operand_{operand} {
    // Transform &*unit -> unit (like in C)
    if (auto op = dynamic_cast<DereferenceExpression*>(operand_)) {
      operand_ = op->operand_;
    }
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitAddressof(this);
  }

  virtual types::Type* GetType() override {
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return ampersand_.location;
  }

  lex::Token ampersand_;

  /**/ Expression* operand_;

  // Mabye embed and save allocation
  types::Type* type_ = nullptr;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FnCallExpression : public Expression {
 public:
  FnCallExpression(lex::Token paren, Expression* callable,
                   std::vector<Expression*> arguments)
      : paren_(paren), callable_{callable}, arguments_{arguments} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFnCall(this);
  }

  virtual types::Type* GetType() override {
    auto l = types::FindLeader(callable_type_);
    FMT_ASSERT(l->tag == types::TypeTag::TY_FUN, "Typechecker fault");
    return types::FindLeader(l->as_fun.result_type);
  };

  std::string_view GetFunctionName() {
    return fn_name_;
  };

  virtual lex::Location GetLocation() override {
    return paren_.location;
  }

  lex::Token paren_;

  // May be absent
  std::string_view fn_name_;

  Expression* callable_;
  types::Type* callable_type_ = nullptr;

  std::vector<Expression*> arguments_;

  bool is_tail_call_ = false;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class IntrinsicCall : public FnCallExpression {
 public:
  IntrinsicCall(FnCallExpression* node) : FnCallExpression(std::move(*node)) {
    intrinsic = ast::elaboration::intrinsics_table.at(node->GetFunctionName());
  }

  virtual types::Type* GetType() override {
    switch (intrinsic) {
      case ast::elaboration::Intrinsic::PRINT:
        return &types::builtin_unit;

      case ast::elaboration::Intrinsic::ASSERT:
        return &types::builtin_unit;

      case ast::elaboration::Intrinsic::IS_NULL:
        return &types::builtin_bool;

      default:
        std::abort();
    }
  };

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitIntrinsic(this);
  }

  ast::elaboration::Intrinsic intrinsic;
};

//////////////////////////////////////////////////////////////////////

// At least for now let's call it that
class CompoundInitializerExpr : public Expression {
 public:
  struct Member {
    std::string_view field;
    Expression* init;
  };

  CompoundInitializerExpr(lex::Token curly, std::vector<Member> values)
      : curly_{curly}, initializers_{values} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitCompoundInitalizer(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Oh-huh!");
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return curly_.location;
  }

  lex::Token curly_;

  std::vector<Member> initializers_;

  types::Type* type_ = nullptr;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FieldAccessExpression : public Expression {
 public:
  FieldAccessExpression(lex::Token field_name, Expression* lvalue)
      : struct_expression_{lvalue}, field_name_{field_name} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFieldAccess(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Returning unassigned type");
    return types::FindLeader(type_);
  };

  std::string_view GetFieldName() {
    return field_name_.GetName();
  }

  virtual lex::Location GetLocation() override {
    return field_name_.location;
  }

  // This can be an Identifier or or result of
  // indexing an array, or of a field access.
  Expression* struct_expression_;

  types::Type* type_ = nullptr;

  lex::Token field_name_;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class BlockExpression : public Expression {
 public:
  BlockExpression(lex::Token curly_brace, Expression* expr)
      : curly_brace_{curly_brace}, expr_{expr} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitBlock(this);
  }

  virtual types::Type* GetType() override {
    return expr_->GetType();
  };

  virtual lex::Location GetLocation() override {
    return curly_brace_.location;
  }

  lex::Token curly_brace_;

  Expression* expr_;

  ast::scope::ScopeLayer* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class LiteralExpression : public Expression {
 public:
  LiteralExpression(lex::Token token) : token_{token} {
  }

  LiteralExpression(const LiteralExpression& other) = default;

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitLiteral(this);
  }

  virtual types::Type* GetType() override {
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return token_.location;
  }

  types::Type* type_ = nullptr;

  lex::Token token_;
};

//////////////////////////////////////////////////////////////////////

class IfExpression : public Expression {
 public:
  IfExpression(Expression* condition, Expression* true_branch,
               Expression* false_branch)
      : condition_{condition},
        true_branch_{true_branch},
        false_branch_{false_branch} {
    false_branch_ = false_branch_
                        ? false_branch_
                        : new LiteralExpression{lex::Token::UnitToken()};
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitIf(this);
  }

  virtual types::Type* GetType() override {
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return condition_->GetLocation();
  }

  Expression* condition_;

  types::Type* type_ = nullptr;

  Expression* true_branch_;
  Expression* false_branch_;
};

//////////////////////////////////////////////////////////////////////

class MatchExpression : public Expression {
 public:
  using Bind = std::pair<Pattern*, Expression*>;

  MatchExpression(Expression* against, std::vector<Bind> patterns)
      : against_(against), patterns_(std::move(patterns)) {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitMatch(this);
  }

  virtual types::Type* GetType() override {
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return against_->GetLocation();
  }

  Expression* against_;

  types::Type* type_ = nullptr;
  std::vector<Bind> patterns_;
};

//////////////////////////////////////////////////////////////////////

class NewExpression : public Expression {
 public:
  NewExpression(lex::Token new_token, Expression* allocation_size,
                Expression* initial_value, types::Type* underlying)
      : new_token_{new_token},
        allocation_size_{allocation_size},
        initial_value_{initial_value},
        underlying_{underlying} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitNew(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Not set type");
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return new_token_.location;
  }

  lex::Token new_token_{};

  Expression* allocation_size_ = nullptr;

  Expression* initial_value_ = nullptr;

  types::Type* underlying_ = nullptr;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class VarAccessExpression : public Expression {
 public:
  VarAccessExpression(lex::Token name) : name_{name} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarAccess(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Type field is not set!");
    return types::FindLeader(type_);
  };

  std::string_view GetName() {
    return name_.GetName();
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  lex::Token name_;

  types::Type* type_ = nullptr;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class TypecastExpression : public Expression {
 public:
  TypecastExpression(Expression* expr, lex::Token flowy_arrow,
                     types::Type* dest)
      : expr_{expr}, flowy_arrow_{flowy_arrow}, type_{dest} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitTypecast(this);
  }

  virtual types::Type* GetType() override {
    FMT_ASSERT(type_, "Type field is not set!");
    return types::FindLeader(type_);
  };

  virtual lex::Location GetLocation() override {
    return flowy_arrow_.location;
  }

  Expression* expr_ = nullptr;

  lex::Token flowy_arrow_;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class ReturnExpression : public Expression {
 public:
  ReturnExpression(lex::Token return_token, Expression* return_value)
      : return_token_{return_token}, return_value_{return_value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitReturn(this);
  }

  virtual types::Type* GetType() override {
    return &types::builtin_never;
  }

  virtual lex::Location GetLocation() override {
    return return_token_.location;
  }

  lex::Token return_token_;
  Expression* return_value_;

  std::string_view this_fun;
  ast::scope::Context* layer_;
};

//////////////////////////////////////////////////////////////////////

class YieldExpression : public Expression {
 public:
  YieldExpression(lex::Token yield_token, Expression* yield_value)
      : yield_token_{yield_token}, yield_value_{yield_value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitYield(this);
  }

  virtual types::Type* GetType() override {
    return &types::builtin_never;
  }

  virtual lex::Location GetLocation() override {
    return yield_token_.location;
  }

  lex::Token yield_token_;
  Expression* yield_value_;
};

//////////////////////////////////////////////////////////////////////

class IndexExpression : public Expression {
 public:
  IndexExpression(lex::Token bracket, Expression* indexed, Expression* index)
      : indexed_expr_{indexed}, square_bracket_{bracket}, index_{index} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitIndex(this);
  }

  virtual types::Type* GetType() override {
    return type_->leader;
  }

  virtual lex::Location GetLocation() override {
    return square_bracket_.location;
  }

  Expression* indexed_expr_;
  lex::Token square_bracket_;
  Expression* index_;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class SeqExpression : public Expression {
 public:
  SeqExpression(Expression* expr, lex::Token semicolon, Expression* rest)
      : expr_{expr}, semicolon_{semicolon}, rest_(rest) {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitSeqExpr(this);
  }

  virtual types::Type* GetType() override {
    return rest_ ? rest_->GetType() : &types::builtin_unit;
    // TODO: perf?
  }

  virtual lex::Location GetLocation() override {
    return expr_->GetLocation();
  }

  Expression* expr_;

  lex::Token semicolon_;

  Expression* rest_;
};

//////////////////////////////////////////////////////////////////////

class LetExpression : public Expression {
 public:
  LetExpression(lex::Token let, Pattern* pat, Expression* value,
                Expression* _else, Expression* rest)
      : let_(let),
        pattern_(pat),
        value_(value),
        rest_(rest),
        else_rest_(_else) {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitLet(this);
  }

  virtual types::Type* GetType() override {
    return types::FindLeader(type_);
  }

  virtual lex::Location GetLocation() override {
    return let_.location;
  }

  lex::Token let_;
  Pattern* pattern_;

  Expression* value_;

  Expression* rest_;
  Expression* else_rest_;

  types::Type* type_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class AssignExpression : public Expression {
 public:
  AssignExpression(lex::Token assign, Expression* target, Expression* value)
      : assign_{assign}, target_{target}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitAssign(this);
  }

  virtual types::Type* GetType() override {
    return &types::builtin_unit;
  }

  virtual lex::Location GetLocation() override {
    return assign_.location;
  }

  lex::Token assign_;

  Expression* target_;
  Expression* value_;
};

//////////////////////////////////////////////////////////////////////
