#include <vm/interpreter.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Return exit code", "[vm]") {
  vm::BytecodeInterpreter interpreter;

  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{.type = vm::InstrType::PUSH_STACK,  //
                        .arg1 = 0}                          //
          },                                                //
      //
      .attached_vals{vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                            .as_int = 123}},
  };

  CHECK(interpreter.Interpret(&chunk) == 123);
}

//////////////////////////////////////////////////////////////////////
