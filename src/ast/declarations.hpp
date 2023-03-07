#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/expressions.hpp>

#include <types/type.hpp>

#include <lex/token.hpp>

#include <vector>

//////////////////////////////////////////////////////////////////////

class Declaration : public TreeNode {
 public:
  virtual void Accept(Visitor* /* visitor */){};

  virtual std::string_view GetName() = 0;
};

//////////////////////////////////////////////////////////////////////

class TraitDeclaration : public Declaration {
 public:
  TraitDeclaration(lex::Token name, std::vector<lex::Token> params,
                   std::vector<Declaration*> items)
      : name_{name},
        parameters_{std::move(params)},
        assoc_items_{std::move(items)} {
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

  std::vector<Declaration*> assoc_items_;
};

//////////////////////////////////////////////////////////////////////

class ImplDeclaration : public Declaration {
 public:
  ImplDeclaration(lex::Token name, types::Type* for_type,
                  std::vector<types::Type*> params,
                  std::vector<Declaration*> assoc_items)
      : trait_name_{name},
        for_type_{for_type},
        params_{std::move(params)},
        assoc_items_{std::move(assoc_items)} {
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

  std::vector<Declaration*> assoc_items_;
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
  VarDeclaration(lex::Token name, Expression* value, types::Type* hint)
      : name_{name}, annotation_{hint}, value_{value} {
  }

  virtual void Accept(Visitor* visitor) override {
    visitor->VisitVarDecl(this);
  }

  virtual lex::Location GetLocation() override {
    return name_.location;
  }

  std::string_view GetName() override {
    return name_.GetName();
  }

  // var or static
  lex::Token type_;

  // Specific type for GetName method
  lex::Token name_;

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

  lex::Token name_;

  Attribute* attributes = nullptr;

  types::Type* type_ = nullptr;

  std::vector<lex::Token> formals_;

  Expression* body_;

  ast::scope::Context* layer_ = nullptr;
};

//////////////////////////////////////////////////////////////////////
