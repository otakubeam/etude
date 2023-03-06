#include <parse/parser.hpp>

///////////////////////////////////////////////////////////////////

auto Parser::ParseModule() -> Module {
  ModuleBuilder builder{*this};
  return builder.Run();
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::Run() -> Module {
  ParseImports();
  ParseExportBlock();
  ParseRestDefinitions();
  result_.ExportTraitMethods();
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::ParseImports() -> void {
  while (me_.Matches(lex::TokenType::IDENTIFIER)) {
    auto modulename = me_.lexer_.GetPreviousToken();
    result_.imports_.push_back(modulename);
    me_.Consume(lex::TokenType::SEMICOLON);
  }
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::ParseExportBlock() -> void {
  if (!me_.Matches(lex::TokenType::EXPORT)) {
    return;
  }

  me_.Consume(lex::TokenType::LEFT_CBRACE);

  while (!me_.Matches(lex::TokenType::RIGHT_CBRACE)) {
    auto proto = me_.ParseDeclaration();
    result_.exported_.push_back(proto->GetName());
    result_.items_.push_back(proto);

  }
}

///////////////////////////////////////////////////////////////////

auto Parser::ModuleBuilder::ParseRestDefinitions() -> void {
  auto declarations = std::vector<Declaration*>{};

  while (!me_.Matches(lex::TokenType::TOKEN_EOF)) {
    auto declaration = me_.ParseDeclaration();
    result_.items_.push_back(declaration);
  }
}

///////////////////////////////////////////////////////////////////
