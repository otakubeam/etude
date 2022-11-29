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

  auto ParseExternBlock = [this]() -> auto{
    std::vector<Declaration*> externs;

    if (!Matches(lex::TokenType::EXTERN)) {
      return externs;
    }

    Consume(lex::TokenType::RIGHT_CBRACE);

    while (auto decl = ParseDeclaration()) {
      externs.push_back(decl);
    }

    Consume(lex::TokenType::LEFT_CBRACE);

    return externs;
  };

  auto externs = ParseExternBlock();

  // 3. Parse export block
  // ---------------------

  auto ParseExportBlock = [this]() -> auto{
    std::unordered_map<std::string_view, Declaration*> exported;

    if (!Matches(lex::TokenType::EXPORT)) {
      return exported;
    }

    Consume(lex::TokenType::RIGHT_CBRACE);

    while (auto decl = ParseDeclaration()) {
      exported.insert({decl->GetName(), decl});
    }

    Consume(lex::TokenType::LEFT_CBRACE);

    return exported;
  };

  result.exported_syms_ = ParseExportBlock();

  // 4. Parse the rest of definitions
  // --------------------------------

  auto declarations = std::vector<Declaration*>{};

  while (auto declaration = ParseDeclaration()) {
    declarations.push_back(declaration);
  }

  Consume(lex::TokenType::TOKEN_EOF);

  result.items_ = std::move(declarations);

  return result;
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
  if (!Matches(lex::TokenType::FUN)) {
    return nullptr;
  }

  auto fun_name = lexer_.Peek();
  Consume(lex::TokenType::IDENTIFIER);

  auto formals = ParseFormals();

  // Funtion prototype

  if (Matches(lex::TokenType::SEMICOLON)) {
    return new FunDeclStatement{fun_name, std::move(formals), nullptr, hint};
  };

  // Funtion definition

  Consume(lex::TokenType::ASSIGN);

  auto body = ParseExpression();

  Consume(lex::TokenType::SEMICOLON);

  return new FunDeclStatement{fun_name, std::move(formals), body, hint};
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
      // case lex::TokenType::STATIC:
      lexer_.Advance();
      type = lexer_.GetPreviousToken();
      break;

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
