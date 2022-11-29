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
      decl->is_extern_ = true;
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
      proto->is_exported_ = true;
    }

    return exported;
  };

  result.exported_ = ParseExportBlock();

  // 4. Parse the rest of definitions
  // --------------------------------

  auto declarations = std::vector<Declaration*>{};

  while (!Matches(lex::TokenType::TOKEN_EOF)) {
    auto declaration = ParseDeclaration();
    declarations.push_back(declaration);
  }

  result.items_ = std::move(declarations);

  return result;
}

///////////////////////////////////////////////////////////////////

Declaration* Parser::ParsePrototype(bool) {
  if (auto type_declaration = ParseTypeDeclStatement()) {
    return type_declaration;
  }

  Consume(lex::TokenType::OF);

  auto hint = ParseFunctionType();

  if (auto fun_proto = ParseFunPrototype(hint)) {
    Consume(lex::TokenType::SEMICOLON);
    return fun_proto;
  }

  std::abort();
}

FunDeclStatement* Parser::ParseFunPrototype(types::Type* hint) {
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto fun_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto formals = ParseFormals();

  return new FunDeclStatement{fun_name, std::move(formals), nullptr, hint};
}

///////////////////////////////////////////////////////////////////

Declaration* Parser::ParseDeclaration() {
  types::Type* hint = nullptr;
  if (Matches(lex::TokenType::OF)) {
    hint = ParseFunctionType();
  }

  if (auto type_declaration = ParseTypeDeclStatement()) {
    return type_declaration;
  }

  if (auto var_declaration = ParseVarDeclStatement(hint)) {
    return var_declaration;
  }

  if (auto fun_declaration = ParseFunDeclStatement(hint)) {
    return fun_declaration;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////

FunDeclStatement* Parser::ParseFunDeclStatement(types::Type* hint) {
  auto proto = ParseFunPrototype(hint);

  if (Matches(lex::TokenType::SEMICOLON)) {
    return proto;
  };

  // Funtion definition

  Consume(lex::TokenType::ASSIGN);

  proto->body_ = ParseExpression();

  Consume(lex::TokenType::SEMICOLON);

  return proto;
}

///////////////////////////////////////////////////////////////////

TypeDeclStatement* Parser::ParseTypeDeclStatement() {
  if (!Matches(lex::TokenType::TYPE)) {
    return nullptr;
  }

  auto type_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto formals = ParseFormals();

  // Type declaration

  if (Matches(lex::TokenType::SEMICOLON)) {
    return new TypeDeclStatement{type_name, std::move(formals), nullptr};
  };

  // Typedefinition definition

  Consume(lex::TokenType::ASSIGN);

  auto body = ParseFunctionType();

  Consume(lex::TokenType::SEMICOLON);

  return new TypeDeclStatement{type_name, std::move(formals), body};
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

VarDeclStatement* Parser::ParseVarDeclStatement(types::Type* hint) {
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

  return new VarDeclStatement{lvalue, value, hint};
}

///////////////////////////////////////////////////////////////////
