#include <rt/scope/environment.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Scoping", "[rt]") {
  Environment global = Environment::MakeGlobal();

  Environment* interpreter_scope = &global;
  interpreter_scope->Declare("a", FromPrim(3));
  CHECK(interpreter_scope->Get("a") == FromPrim(3));

  {
    Environment::ScopeGuard guard1{&interpreter_scope};

    // Shadow global a

    interpreter_scope->Declare("a", FromPrim(5));
    CHECK(interpreter_scope->Get("a") == FromPrim(5));

    {
      Environment::ScopeGuard guard2{&interpreter_scope};

      // Assign to a from the first outer scope

      interpreter_scope->TryAssign("a", FromPrim(9));
    }

    CHECK(interpreter_scope->Get("a") == FromPrim(9));
  }

  CHECK(interpreter_scope->Get("a") == FromPrim(3));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Shadowing", "[rt]") {
  Environment global = Environment::MakeGlobal();

  Environment* interpreter_scope = &global;
  interpreter_scope->Declare("a", FromPrim(3));

  {
    Environment::ScopeGuard guard1{&interpreter_scope};
    CHECK(interpreter_scope->Get("a") == FromPrim(3));

    // Shadow

    interpreter_scope->Declare("a", FromPrim(9));
    CHECK(interpreter_scope->Get("a") == FromPrim(9));
  }

  CHECK(interpreter_scope->Get("a") == FromPrim(3));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Assign", "[rt]") {
  Environment global = Environment::MakeGlobal();

  Environment* interpreter_scope = &global;
  interpreter_scope->Declare("a", FromPrim(3));

  {
    Environment::ScopeGuard guard1{&interpreter_scope};

    // Insert
    interpreter_scope->Declare("b", FromPrim(4));

    // Assign
    interpreter_scope->TryAssign("a", FromPrim(4));
  }

  CHECK(interpreter_scope->Get("a") == FromPrim(4));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Get", "[rt]") {
  Environment global = Environment::MakeGlobal();

  Environment* interpreter_scope = &global;
  interpreter_scope->Declare("a", FromPrim(3));

  {
    Environment::ScopeGuard guard1{&interpreter_scope};
    interpreter_scope->Declare("b", FromPrim(4));

    {
      Environment::ScopeGuard guard2{&interpreter_scope};
      interpreter_scope->Declare("c", FromPrim(5));
      CHECK_NOTHROW(interpreter_scope->Get("a").value());
      CHECK_NOTHROW(interpreter_scope->Get("b").value());
      CHECK_NOTHROW(interpreter_scope->Get("c").value());
    }

    CHECK_NOTHROW(interpreter_scope->Get("a").value());
    CHECK_NOTHROW(interpreter_scope->Get("b").value());
    CHECK_THROWS(interpreter_scope->Get("c").value());
  }

  CHECK_NOTHROW(interpreter_scope->Get("a").value());
  CHECK_THROWS(interpreter_scope->Get("b").value());
  CHECK_THROWS(interpreter_scope->Get("c").value());
}

//////////////////////////////////////////////////////////////////////
