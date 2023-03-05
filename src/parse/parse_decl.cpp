#include <parse/parser.hpp>
#include <parse/parse_error.hpp>

///////////////////////////////////////////////////////////////////

auto Parser::ParseModule() -> Module {
  Module result;

  // 1. Parse imported modules;
  // --------------------------

  auto ParseImports = [this, &result]() {
    while (Matches(lex::TokenType::IDENTIFIER)) {
      result.imports_.push_back(lexer_.GetPreviousToken());
      Consume(lex::TokenType::SEMICOLON);
    }
  };

  ParseImports();

  // 2. Parse optional extern block
  // ------------------------------

  auto ParseExternBlock = [ this, &result ]() -> auto{
    if (!Matches(lex::TokenType::EXTERN)) {
      return;
    }

    Consume(lex::TokenType::RIGHT_CBRACE);

    while (auto decl = ParseDeclaration()) {
      result.items_.push_back(decl);
    }

    Consume(lex::TokenType::LEFT_CBRACE);
  };

  ParseExternBlock();

  // 3. Parse export block
  // ---------------------

  auto ParseExportBlock = [ this, &result ]() -> auto{
    std::vector<std::string_view> exported;

    if (!Matches(lex::TokenType::EXPORT)) {
      return exported;
    }

    Consume(lex::TokenType::LEFT_CBRACE);

    while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
      auto proto = ParsePrototype();
      exported.push_back(proto->GetName());
      result.items_.push_back(proto);
      if (auto trait = proto->as<TraitDeclaration>()) {
        for (auto method : trait->methods_) {
          exported.push_back(method->GetName());
        }
      }
    }

    return exported;
  };

  result.exported_ = ParseExportBlock();

  // 4. Parse the rest of definitions
  // --------------------------------

  auto declarations = std::vector<Declaration*>{};

  while (!Matches(lex::TokenType::TOKEN_EOF)) {
    auto declaration = ParseDeclaration();

    result.items_.push_back(declaration);

    if (auto fun = declaration->as<FunDeclaration>()) {
      if (fun->attributes && fun->attributes->FindAttr("test")) {
        result.tests_.push_back(fun);
      }
    }
  }

  return result;
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

Declaration* Parser::ParsePrototype(bool) {
  if (auto type_declaration = ParseTypeDeclaration()) {
    return type_declaration;
  }

  if (auto trait_declaration = ParseTraitDeclaration()) {
    return trait_declaration;
  }

  Consume(lex::TokenType::OF);

  auto hint = ParseFunctionType();

  auto attrs = ParseAttributes();

  if (auto fun_proto = ParseFunPrototype(hint)) {
    Consume(lex::TokenType::SEMICOLON);
    fun_proto->attributes = attrs;
    return fun_proto;
  }

  std::abort();
}

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

FunDeclaration* Parser::ParseFunDeclarationStandalone() {
  Consume(lex::TokenType::OF);
  auto hint = ParseFunctionType();
  auto result = ParseFunPrototype(hint);
  Consume(lex::TokenType::SEMICOLON);
  return result;
}

///////////////////////////////////////////////////////////////////

Declaration* Parser::ParseDeclaration() {
  types::Type* hint = nullptr;
  if (Matches(lex::TokenType::OF)) {
    hint = ParseFunctionType();
  }

  if (auto type_declaration = ParseTypeDeclaration()) {
    return type_declaration;
  }

  if (auto var_declaration = ParseVarDeclaration(hint)) {
    return var_declaration;
  }

  if (auto trait_declaration = ParseTraitDeclaration()) {
    return trait_declaration;
  }

  if (auto impl_declaration = ParseImplDeclaration()) {
    return impl_declaration;
  }

  if (auto fun_declaration = ParseFunDeclaration(hint)) {
    return fun_declaration;
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

  proto->attributes = attrs;

  // Funtion definition

  Consume(lex::TokenType::ASSIGN);

  proto->body_ = ParseExpression();

  Consume(lex::TokenType::SEMICOLON);

  return proto;
}

///////////////////////////////////////////////////////////////////

ImplDeclaration* Parser::ParseImplDeclaration() {
  if (!Matches(lex::TokenType::IMPL)) {
    return nullptr;
  }

  auto trait_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  std::vector<types::Type*> type_params_;

  while (!Matches(lex::TokenType::FOR)) {
    type_params_.push_back(ParseType());
  }

  auto for_type = ParseType();

  Consume(lex::TokenType::LEFT_CBRACE);

  std::vector<FunDeclaration*> definitions;
  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    types::Type* hint = nullptr;

    if (Matches(lex::TokenType::OF)) {
      hint = ParseFunctionType();
    }

    definitions.push_back(ParseFunDeclaration(hint));
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

  std::vector<FunDeclaration*> trait_methods;

  while (!Matches(lex::TokenType::RIGHT_CBRACE)) {
    if (auto method = ParseFunDeclarationStandalone()) {
      trait_methods.push_back(method);
    }
  }

  return new TraitDeclaration{name, std::move(parameters),
                              std::move(trait_methods)};
}

///////////////////////////////////////////////////////////////////

TypeDeclaration* Parser::ParseTypeDeclaration() {
  if (!Matches(lex::TokenType::TYPE)) {
    return nullptr;
  }

  auto type_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto formals = ParseFormals();

  // Type declaration

  if (Matches(lex::TokenType::SEMICOLON)) {
    return new TypeDeclaration{type_name, std::move(formals), nullptr};
  };

  // Typedefinition definition

  Consume(lex::TokenType::ASSIGN);

  auto body = ParseFunctionType();

  Consume(lex::TokenType::SEMICOLON);

  return new TypeDeclaration{type_name, std::move(formals), body};
}

///////////////////////////////////////////////////////////////////

auto Parser::ParseFormals() -> std::vector<lex::Token> {
  std::vector<lex::Token> result;

  while (Matches(lex::TokenType::IDENTIFIER)) {
    result.push_back(lexer_.GetPreviousToken());
  }

  return result;
}

///////////////////////////////////////////////////////////////////

VarDeclaration* Parser::ParseVarDeclaration(types::Type* hint) {
  lex::Token type;
  switch (lexer_.Peek().type) {
    case lex::TokenType::VAR:
      lexer_.Advance();
      type = lexer_.GetPreviousToken();
      break;

      // case lex::TokenType::STATIC:

    default:
      return nullptr;
  }

  // 1. Get a name to assign to

  Consume(lex::TokenType::IDENTIFIER);
  auto lvalue = new VarAccessExpression{lexer_.GetPreviousToken()};

  // 2. Get an expression to assign to

  Consume(lex::TokenType::ASSIGN);

  auto value = ParseExpression();

  Consume(lex::TokenType::SEMICOLON);

  return new VarDeclaration{lvalue, value, hint};
}

///////////////////////////////////////////////////////////////////
