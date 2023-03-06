#include <parse/parser.hpp>

///////////////////////////////////////////////////////////////////

Declaration* Parser::ParseDeclaration() {
  if (auto assoc_declaration = ParseAssociatedItems()) {
    return assoc_declaration;
  }

  if (auto trait_declaration = ParseTraitDeclaration()) {
    return trait_declaration;
  }

  if (auto impl_declaration = ParseImplDeclaration()) {
    return impl_declaration;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////

Declaration* Parser::ParseAssociatedItems() {
  types::Type* hint = nullptr;

  if (Matches(lex::TokenType::OF)) {
    hint = ParseFunctionType();
  }

  if (auto fun_declaration = ParseFunDeclaration(hint)) {
    return fun_declaration;
  }

  if (auto var_declaration = ParseVarDeclaration(hint)) {
    return var_declaration;
  }

  if (auto type_declaration = ParseTypeDeclaration()) {
    return type_declaration;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////

FunDeclaration* Parser::ParseFunDeclaration(types::Type* hint) {
  auto attrs = ParseAttributes();
  auto proto = ParseFunPrototype(hint);

  if (!proto || Matches(lex::TokenType::SEMICOLON)) {
    return proto;
  };

  proto->attributes = attrs;  // TODO: flesh out attributes
  Consume(lex::TokenType::ASSIGN);

  proto->body_ = ParseExpression();
  Consume(lex::TokenType::SEMICOLON);

  return proto;
}

///////////////////////////////////////////////////////////////////

Attribute* Parser::ParseAttributes() {
  Attribute* attr = nullptr;

  while (Matches(lex::TokenType::ATTRIBUTE)) {
    Consume(lex::TokenType::IDENTIFIER);
    auto value = lexer_.GetPreviousToken();
    attr ? attr->next : attr = new Attribute{value.GetName()};
  }

  return attr;
}

///////////////////////////////////////////////////////////////////

FunDeclaration* Parser::ParseFunPrototype(types::Type* hint) {
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto fun_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto formals = ParseFormals();

  return new FunDeclaration{fun_name, std::move(formals), nullptr, hint};
}

///////////////////////////////////////////////////////////////////

VarDeclaration* Parser::ParseVarDeclaration(types::Type* hint) {
  if (!Matches(lex::TokenType::VAR)) {
    return nullptr;
  }

  Consume(lex::TokenType::IDENTIFIER);

  auto name = lexer_.GetPreviousToken();
  Consume(lex::TokenType::ASSIGN);

  auto value = ParseExpression();
  Consume(lex::TokenType::SEMICOLON);

  return new VarDeclaration{name, value, hint};
}

///////////////////////////////////////////////////////////////////

ImplDeclaration* Parser::ParseImplDeclaration() {
  if (!Matches(lex::TokenType::IMPL)) {
    return nullptr;
  }

  // Parse Impl Header

  auto trait_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  std::vector<types::Type*> type_params_;

  while (!Matches(lex::TokenType::FOR)) {
    type_params_.push_back(ParseType());
  }

  auto for_type = ParseType();

  if (Matches(lex::TokenType::WHERE)) {
    // Parse ImplWhere clause
  }

  Consume(lex::TokenType::LEFT_CBRACE);

  // Parse Impl Definitions

  std::vector<Declaration*> definitions;

  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    definitions.push_back(ParseAssociatedItems());
  }

  return new ImplDeclaration(trait_name, for_type, std::move(type_params_),
                             std::move(definitions));
}

///////////////////////////////////////////////////////////////////

TraitDeclaration* Parser::ParseTraitDeclaration() {
  if (!Matches(lex::TokenType::TRAIT)) {
    return nullptr;
  }

  auto name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto parameters = ParseFormals();
  Consume(lex::TokenType::LEFT_CBRACE);

  std::vector<Declaration*> trait_methods;

  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    if (auto method = ParseAssociatedItems()) {
      trait_methods.push_back(method);
    }
  }

  return new TraitDeclaration{name, std::move(parameters),
                              std::move(trait_methods)};
}

///////////////////////////////////////////////////////////////////

FunDeclaration* Parser::ParseFunDeclarationStandalone() {
  Consume(lex::TokenType::OF);
  auto hint = ParseFunctionType();
  auto result = ParseFunPrototype(hint);
  Consume(lex::TokenType::SEMICOLON);
  return result;
}

///////////////////////////////////////////////////////////////////

TypeDeclaration* Parser::ParseTypeDeclaration() {
  if (!Matches(lex::TokenType::TYPE)) {
    return nullptr;
  }

  auto type_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto formals = ParseFormals();
  Consume(lex::TokenType::ASSIGN);

  auto body = ParseFunctionType();
  Consume(lex::TokenType::SEMICOLON);

  return new TypeDeclaration{type_name, std::move(formals), body};
}

///////////////////////////////////////////////////////////////////
