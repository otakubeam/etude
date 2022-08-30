#include <vm/interpreter.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("Return exit code", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{.type = vm::InstrType::PUSH_STACK,  //
                        .arg1 = 0}                          //
          },                                                //
      //
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 123},                //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(&chunk) == 123);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Push and pop", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{.type = vm::InstrType::PUSH_STACK,  //
                        .arg1 = 0},                         //
              vm::Instr{.type = vm::InstrType::PUSH_STACK,  //
                        .arg1 = 1},                         //
              vm::Instr{.type = vm::InstrType::POP_STACK},  //
          },                                                //
      //
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,   //
                                 .as_int = 123},                 //
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Bool,  //
                                 .as_bool = true},               //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(&chunk) == 123);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("JUMP_IF_FALSE instr", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push 100
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 1,  // push 500
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 2,  // push false
              },
              vm::Instr{
                  .type = vm::InstrType::JUMP_IF_FALSE,
                  .arg2 = 0,
                  .arg3 = 5,  // skip next instr
              },
              vm::Instr{
                  .type = vm::InstrType::POP_STACK,
              },
          },
      //
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,   //
                                 .as_int = 100},                 //
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,   //
                                 .as_int = 500},                 //
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Bool,  //
                                 .as_bool = false},              //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(&chunk) == 500);
}

//////////////////////////////////////////////////////////////////////
