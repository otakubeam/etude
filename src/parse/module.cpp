#include <parse/parser.hpp>

///////////////////////////////////////////////////////////////////

auto Parser::ParseModule(std::string_view name) -> ModuleDeclaration* {
  ModuleBuilder builder{*this};
  auto result = builder.Run();
  result->name_ = name;
  return result;
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::Run() -> ModuleDeclaration* {
  result = new ModuleDeclaration;
  ParseImports();
  ParseExportBlock();
  ParseRestDefinitions();
  // result_.ExportTraitMethods();
  return result;
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::ParseImports() -> void {
  while (me.Matches(lex::TokenType::IDENTIFIER)) {
    auto modulename = me.lexer_.GetPreviousToken();
    result->imports_.push_back(modulename);
    me.Consume(lex::TokenType::SEMICOLON);
  }
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::ParseExportBlock() -> void {
  if (!me.Matches(lex::TokenType::EXPORT)) {
    return;
  }

  me.Consume(lex::TokenType::LEFT_CBRACE);

  while (!me.Matches(lex::TokenType::RIGHT_CBRACE)) {
    auto declaration = me.ParseDeclaration();
    result->exported_.push_back(declaration);
  }
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::ParseRestDefinitions() -> void {
  auto declarations = std::vector<Declaration*>{};

  while (!me.Matches(lex::TokenType::TOKEN_EOF)) {
    auto declaration = me.ParseDeclaration();
    result->local_.push_back(declaration);
  }
}

///////////////////////////////////////////////////////////////////
