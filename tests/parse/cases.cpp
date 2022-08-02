#include <parse/parser.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parser: Just works", "[parser]") {
  char stream[] = "1 - 2";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(BinaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse as separate", "[parser]") {
  char stream[] = "1 - 2";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParsePrimary();
  REQUIRE(typeid(*expr) == typeid(LiteralExpression));

  expr = p.ParseUnary();
  REQUIRE(typeid(*expr) == typeid(UnaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parser: Booleans", "[parser]") {
  char stream[] = "!true";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(UnaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Variable declaration", "[parser]") {
  char stream[] = "var x = 5;";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(VarDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Misleading minus", "[parser]") {
  char stream[] = "- 1 - 2";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(BinaryExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("No left bracket", "[parser]") {
  char stream[] = "1 - (1 + 2";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};
  CHECK_THROWS(p.ParseExpression());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("No braced expression", "[parser]") {
  char stream[] = "()";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};
  CHECK_THROWS(p.ParseExpression());
}

//////////////////////////////////////////////////////////////////////
//                           Statements
//////////////////////////////////////////////////////////////////////

TEST_CASE("Expression statement", "[parser]") {
  char stream[] = "1 + 2;";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(ExprStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse string literal", "[parser]") {
  char stream[] = " \"a\" + \"b\" ";
  //                -----   -----
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};
  CHECK_NOTHROW(p.ParseExpression());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse string literal (II)", "[parser]") {
  char stream[] = "\"ab\"";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(LiteralExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse function decl", "[parser]") {
  std::stringstream source("fun f  () Unit   { 123; }");
  //                        -----  --------  -------------
  //                        name   no args   expr-statement

  Parser p{lex::Lexer{source}};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(FunDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse complex type", "[parser]") {
  std::stringstream source("((Unit, Int) String, () Bool) Int");
  //                         ------------------  -------- ---

  Parser p{lex::Lexer{source}};

  auto type = p.ParseType();
  REQUIRE(typeid(*type) == typeid(types::FnType));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse function declaration (II)", "[parser]") {
  std::stringstream source(
      "fun f(a1 : Int, a2 : Bool) Unit "
      "{ 1; }");

  Parser p{lex::Lexer{source}};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(FunDeclStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Block statement", "[parser]") {
  std::stringstream source("{ 123; var a = 5; fun f() Unit {}}");

  Parser p{lex::Lexer{source}};
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

TEST_CASE("Variable usage", "[parser]") {
  std::stringstream source("var a = 5; a");

  Parser p{lex::Lexer{source}};
  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(VarDeclStatement));

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(LvalueExpression));
}

//////////////////////////////////////////////////////////////////////

// TEST_CASE("Empty block statement", "[parser]") {
//   std::stringstream source("{}");
//   Parser p{lex::Lexer{source}};
//   auto block_statement = p.ParseBlockStatement();
//   REQUIRE(typeid(*block_statement) == typeid(BlockStatement));
// }

//////////////////////////////////////////////////////////////////////

TEST_CASE("Function application (I)", "[parser]") {
  std::stringstream source("print(4, 3)");
  Parser p{lex::Lexer{source}};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Function application (II)", "[parser]") {
  std::stringstream source("f(g(), h(2))");
  Parser p{lex::Lexer{source}};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Function application (III)", "[parser]") {
  std::stringstream source("f((1 < 2))");
  //                          -------
  //                            bool

  // std::stringstream source("f((1 < 2), \"g\", f)");
  // std::stringstream source("f((1 < 2), \"g\", f)");
  Parser p{lex::Lexer{source}};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Function application (IV)", "[parser]") {
  std::stringstream source("clock()");
  Parser p{lex::Lexer{source}};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Function application (V)", "[parser]") {
  std::stringstream source("f( \"string literal\" )");
  Parser p{lex::Lexer{source}};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Function application (VI)", "[parser]") {
  std::stringstream source("f( f )");
  //                         -----
  //                      higher order
  Parser p{lex::Lexer{source}};

  auto fn_application = p.ParseExpression();
  REQUIRE(typeid(*fn_application) == typeid(FnCallExpression));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse return", "[parser]") {
  std::stringstream source("return foo(123);");
  Parser p{lex::Lexer{source}};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(ReturnStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Parse yield", "[parser]") {
  std::stringstream source("yield;");
  Parser p{lex::Lexer{source}};

  auto stmt = p.ParseStatement();
  REQUIRE(typeid(*stmt) == typeid(YieldStatement));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Block final expr", "[parser]") {
  std::stringstream source("{ 123 }");
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(BlockExpression));
}

TEST_CASE("If Expr", "[parser]") {
  std::stringstream source("if true { 123 } else { false }");
  Parser p{lex::Lexer{source}};

  auto expr = p.ParseExpression();
  REQUIRE(typeid(*expr) == typeid(IfExpression));
}

//////////////////////////////////////////////////////////////////////
