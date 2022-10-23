#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/expressions.hpp>

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

class TypeDeclStatement : public Statement {
 public:
  // of lex::Token -> Vec<_> -> *_
  // -> _
  // TypeDeclStatement name params type
  TypeDeclStatement(lex::Token name, std::vector<lex::Token> params,
                    types::Type* type)
      : name_{name}, parameters_{params}, type_{type} {
  }

  ///////////////////////////////////////////////////////////////////////

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitTypeDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string_view GetStructName() {
    return name_.GetName();
  }

  ///////////////////////////////////////////////////////////////////////

  // auto ZipMembers() -> std::vector<types::StructType::Member> {
  //   std::vector<types::StructType::Member> result;
  //   for (size_t i = 0; i < field_names_.size(); i++) {
  //     result.push_back({
  //         .name = field_names_[i].GetName(),
  //         .type = field_types_[i],
  //     });
  //   }
  //   return result;
  // }

  ///////////////////////////////////////////////////////////////////////

  lex::Token name_;

  // In the future...
  std::vector<lex::Token> parameters_;

  types::Type* type_;
};

//////////////////////////////////////////////////////////////////////

class VarDeclStatement : public Statement {
 public:
  VarDeclStatement(VarAccessExpression* lvalue, Expression* value,
                   types::Type* hint)
      : lvalue_{lvalue}, annotation_{hint}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return lvalue_->GetLocation();
  }

  std::string_view GetVarName() {
    return lvalue_->GetName();
  }

  // var or static
  lex::Token type_;

  // Specific type for GetName method
  VarAccessExpression* lvalue_;

  // Optional, can be inferred from the right part
  types::Type* annotation_ = nullptr;

  Expression* value_;
};

//////////////////////////////////////////////////////////////////////

class FunDeclStatement : public Statement {
 public:
  FunDeclStatement(lex::Token name, std::vector<lex::Token> formals,
                   Expression* body, types::Type* hint)
      : name_{name}, type_{hint}, formals_{std::move(formals)}, body_{body} {
  }

  ///////////////////////////////////////////////////////////////////////

  auto GetArgumentTypes() -> std::vector<types::Type*> {
    std::vector<types::Type*> result;

    return result;
  }

  ///////////////////////////////////////////////////////////////////////

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitFunDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string_view GetFunctionName() {
    return name_.GetName();
  }

  ///////////////////////////////////////////////////////////////////////

  lex::Token name_;

  types::Type* type_ = nullptr;

  std::vector<lex::Token> formals_;

  Expression* body_;

  ast::scope::Context* layer_ = nullptr;
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
