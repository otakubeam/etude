#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:simple", "[parser]") {
  char stream[] = "1 - 2";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(BinaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:boolean", "[parser]") {
  char stream[] = "!true";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(UnaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:vardecl", "[parser]") {
  char stream[] = "var x = 5;";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto stmt = p.ParseDeclaration();
  REQUIRE(typeid(*stmt) == typeid(VarDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:precendence", "[parser]") {
  char stream[] = "- 1 - 2";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(BinaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:error", "[parser]") {
  char stream[] = "1 - (1 + 2";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};
  CHECK_THROWS(p.ParseExpression());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Expression statement", "[parser]") {
  char stream[] = "1 + 2;";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(ExprStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:string-lit-(I)", "[parser]") {
  char stream[] = " \"a\" + \"b\" ";
  //                -----   -----
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};
  CHECK_NOTHROW(p.ParseExpression());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:string-lit-(II)", "[parser]") {
  char stream[] = "\"ab\"";
  std::stringstream source{stream};
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(LiteralExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("paser:fundecl", "[parser]") {
  std::stringstream source("fun f = { 123; };");

  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto stmt = p.ParseDeclaration();
  REQUIRE(typeid(*stmt) == typeid(FunDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:complex-type", "[parser]") {
  std::stringstream source("(Unit -> Int -> *Unit) -> (Bool) -> Int");

  lex::Lexer l{"test.et", source};
  Parser p{l};

  CHECK_NOTHROW(p.ParseType());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("paser:fundecl-(II)", "[parser]") {
  std::stringstream source("fun f x = { x + 1 };");

  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto stmt = p.ParseDeclaration();
  REQUIRE(typeid(*stmt) == typeid(FunDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:block-statement", "[parser]") {
  std::stringstream source("{ 123; var a = 5; fun f = { a }; }");

  lex::Lexer l{"test.et", source};
  Parser p{l};
  auto expr = p.ParseExpression();

  REQUIRE(typeid(*expr) == typeid(BlockExpression));
  BlockExpression* block = dynamic_cast<BlockExpression*>(expr);

  // Explicitly evaluate dereferences

  auto& r1 = *block->stmts_[0];
  auto& r2 = *block->stmts_[1];
  auto& r3 = *block->stmts_[2];

  CHECK(typeid(r1) == typeid(ExprStatement));
  CHECK(typeid(r2) == typeid(VarDeclStatement));
  CHECK(typeid(r3) == typeid(FunDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser::access", "[parser]") {
  std::stringstream source("var a = 5; a");

  lex::Lexer l{"test.et", source};
  Parser p{l};
  auto stmt = p.ParseDeclaration();
  REQUIRE(typeid(*stmt) == typeid(VarDeclStatement));

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(VarAccessExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:block:final", "[parser]") {
  std::stringstream source("{ 123 }");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(BlockExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:empty-block", "[parser]") {
  std::stringstream source("{}");
  lex::Lexer l{"test.et", source};
  Parser p{l};
  auto block_expression = p.ParseBlockExpression();
  REQUIRE(typeid(*block_expression) == typeid(BlockExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:apply-(I)", "[parser]") {
  std::stringstream source("print(4, 3)");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:apply-(II)", "[parser]") {
  std::stringstream source("clock()");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:apply-(III)", "[parser]") {
  std::stringstream source("f(g(), h(2))");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:return-statement", "[parser]") {
  std::stringstream source("return foo(123)");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto stmt = p.ParseExpression();
  REQUIRE(typeid(*stmt) == typeid(ReturnStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:yield", "[parser]") {
  std::stringstream source("yield 123");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto stmt = p.ParseExpression();
  REQUIRE(typeid(*stmt) == typeid(YieldStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:if", "[parser]") {
  std::stringstream source("if true { 123 } else { false }");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(IfExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:struct-decl", "[parser]") {
  std::stringstream source("type S = struct { a: Int, b: Bool, };");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseDeclaration();
  auto expr2 = expr->as<TypeDeclStatement>()->body_;
  REQUIRE(typeid(*expr) == typeid(TypeDeclStatement));
  REQUIRE(expr2->tag == types::TypeTag::TY_STRUCT);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:struct-init", "[parser]") {
  std::stringstream source("{ .i = 123, .b = true, }");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(CompoundInitializerExpr));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:double-deref", "[parser]") {
  std::stringstream source("**ptr");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(DereferenceExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("parser:prec1-chain", "[parser]") {
  std::stringstream source("(&someVar) ~> *Unit ~> *Vec . size");
  lex::Lexer l{"test.et", source};
  Parser p{l};

  auto expr = p.ParseExpression();  // -> size
  REQUIRE(typeid(*expr) == typeid(FieldAccessExpression));

  auto expr2 =
      expr->as<FieldAccessExpression>()->struct_expression_;  // ~> *String
  REQUIRE(typeid(*expr2) == typeid(TypecastExpression));

  auto expr3 = expr2->as<TypecastExpression>()->expr_;  // ~> *Unit
  REQUIRE(typeid(*expr3) == typeid(TypecastExpression));

  auto expr4 = expr3->as<TypecastExpression>()->expr_;  // &someVar
  REQUIRE(typeid(*expr4) == typeid(AddressofExpression));
}

//////////////////////////////////////////////////////////////////////
