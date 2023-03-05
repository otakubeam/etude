#pragma once

#include <ast/statements.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Declaration : public Statement {
 public:
  virtual void Accept(Visitor* /* visitor */){};

  virtual std::string_view GetName() = 0;
};

//////////////////////////////////////////////////////////////////////

class TraitDeclaration : public Declaration {
 public:
  TraitDeclaration(lex::Token name, std::vector<lex::Token> params,
                   std::vector<FunDeclaration*> decls)
      : name_{name},
        parameters_{std::move(params)},
        methods_{std::move(decls)} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitTraitDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string_view GetName() override {
    return name_;
  }

  lex::Token name_;

  std::vector<lex::Token> parameters_;

  std::vector<FunDeclaration*> methods_;

  std::vector<TypeDeclaration*> assoc_types_;

  // This is stupid, but I don't want to make Symbol not POD

  std::vector<ImplDeclaration*> impls_;
};

//////////////////////////////////////////////////////////////////////

class ImplDeclaration : public Declaration {
 public:
  ImplDeclaration(lex::Token name, types::Type* for_type,
                  std::vector<types::Type*> params,
                  std::vector<FunDeclaration*> methods)
      : trait_name_{name},
        for_type_{for_type},
        params_{std::move(params)},
        trait_methods_{std::move(methods)} {
  }

  //  impl Into Str for String  {
  //      fun into self = mk_str(self);
  //  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitImplDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return trait_name_.location;
  }

  std::string_view GetName() override {
    return trait_name_;
  }

  lex::Token trait_name_;

  types::Type* for_type_;

  std::vector<types::Type*> params_;

  std::vector<TypeDeclaration*> assoc_types_;

  std::vector<FunDeclaration*> trait_methods_;
};

//////////////////////////////////////////////////////////////////////

class TypeDeclaration : public Declaration {
 public:
  TypeDeclaration(lex::Token name, std::vector<lex::Token> params,
                    types::Type* body)
      : name_{name}, parameters_{params}, body_{body} {
  }

  ///////////////////////////////////////////////////////////////////////

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitTypeDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string_view GetName() override {
    return name_;
  }

  ///////////////////////////////////////////////////////////////////////

  lex::Token name_;

  bool exported_ = false;

  std::vector<lex::Token> parameters_;

  types::Type* type_;

  types::Type* body_;
};

//////////////////////////////////////////////////////////////////////

class VarDeclaration : public Declaration {
 public:
  VarDeclaration(VarAccessExpression* lvalue, Expression* value,
                   types::Type* hint)
      : lvalue_{lvalue}, annotation_{hint}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return lvalue_->GetLocation();
  }

  std::string_view GetName() override {
    return lvalue_->GetName();
  }

  // var or static
  lex::Token type_;

  // Specific type for GetName method
  VarAccessExpression* lvalue_;

  bool exported_ = false;

  // Optional, can be inferred from the right part
  types::Type* annotation_ = nullptr;

  Expression* value_;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////

class FunDeclaration : public Declaration {
 public:
  FunDeclaration(lex::Token name, std::vector<lex::Token> formals,
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

  std::string_view GetName() override {
    return name_;
  }

  ///////////////////////////////////////////////////////////////////////

  bool trait_method_ = false;

  lex::Token name_;

  Attribute* attributes = nullptr;

  types::Type* type_ = nullptr;

  std::vector<lex::Token> formals_;

  Expression* body_;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////
