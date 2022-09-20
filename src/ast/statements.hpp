#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/expressions.hpp>

#include <types/repr/struct_type.hpp>
#include <types/repr/fn_type.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Statement : public TreeNode {
 public:
  virtual void Accept(Visitor* /* visitor */){};
};

//////////////////////////////////////////////////////////////////////

class ExprStatement : public Statement {
 public:
  ExprStatement(Expression* expr) : expr_{expr} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitExprStatement(this);
  }

  virtual lex::Location GetLocation() override {
    return expr_->GetLocation();
  }

  Expression* expr_;
};

//////////////////////////////////////////////////////////////////////

class StructDeclStatement : public Statement {
 public:
  StructDeclStatement(lex::Token name, std::vector<lex::Token> field_names,
                      std::vector<types::Type*> field_types)
      : name_{name}, field_names_{field_names}, field_types_{field_types} {
    type_ = new types::StructType{name.GetName(), ZipMembers()};
  }

  ///////////////////////////////////////////////////////////////////////

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitStructDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string GetStructName() {
    return name_.GetName();
  }

  ///////////////////////////////////////////////////////////////////////

  auto ZipMembers() -> std::vector<types::StructType::Member> {
    std::vector<types::StructType::Member> result;
    for (size_t i = 0; i < field_names_.size(); i++) {
      result.push_back({
          .name = field_names_[i].GetName(),
          .type = field_types_[i],
      });
    }
    return result;
  }

  ///////////////////////////////////////////////////////////////////////

  lex::Token name_;
  types::StructType* type_;

  std::vector<lex::Token> field_names_;
  std::vector<types::Type*> field_types_;
};

//////////////////////////////////////////////////////////////////////

class VarDeclStatement : public Statement {
 public:
  VarDeclStatement(VarAccessExpression* lvalue, Expression* value)
      : lvalue_{lvalue}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return lvalue_->GetLocation();
  }

  std::string GetVarName() {
    return lvalue_->GetName();
  }

  VarAccessExpression* lvalue_;
  Expression* value_;
};

//////////////////////////////////////////////////////////////////////

class FunDeclStatement : public Statement {
 public:
  struct FormalParam {
    lex::Token ident;
    types::Type* type;

    std::string GetParameterName() {
      return ident.GetName();
    }
  };

  ///////////////////////////////////////////////////////////////////////

  FunDeclStatement(lex::Token name, types::Type* return_type,
                   std::vector<FormalParam> formals, BlockExpression* block)
      : name_{name}, formals_{std::move(formals)}, block_{block} {
    type_ = new types::FnType{GetArgumentTypes(), return_type};
  }

  ///////////////////////////////////////////////////////////////////////

  auto GetArgumentTypes() -> std::vector<types::Type*> {
    std::vector<types::Type*> result;

    for (auto fm : formals_) {
      result.push_back(fm.type);
    }

    return result;
  }

  ///////////////////////////////////////////////////////////////////////

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFunDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string GetFunctionName() {
    return name_.GetName();
  }

  ///////////////////////////////////////////////////////////////////////

  lex::Token name_;
  types::FnType* type_ = nullptr;

  std::vector<FormalParam> formals_;
  BlockExpression* block_;
};

//////////////////////////////////////////////////////////////////////

class ReturnStatement : public Statement {
 public:
  ReturnStatement(lex::Token return_token, Expression* return_value)
      : return_token_{return_token}, return_value_{return_value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitReturn(this);
  }

  virtual lex::Location GetLocation() override {
    return return_token_.location;
  }

  lex::Token return_token_;
  Expression* return_value_;
};

//////////////////////////////////////////////////////////////////////

class YieldStatement : public Statement {
 public:
  YieldStatement(lex::Token yield_token, Expression* yield_value)
      : yield_token_{yield_token}, yield_value_{yield_value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitYield(this);
  }

  virtual lex::Location GetLocation() override {
    return yield_token_.location;
  }

  lex::Token yield_token_;
  Expression* yield_value_;
};

//////////////////////////////////////////////////////////////////////

class AssignmentStatement : public Statement {
 public:
  AssignmentStatement(lex::Token assign, LvalueExpression* target,
                      Expression* value)
      : assign_{assign}, target_{target}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitAssignment(this);
  }

  virtual lex::Location GetLocation() override {
    return assign_.location;
  }

  lex::Token assign_;

  LvalueExpression* target_;

  Expression* value_;
};
