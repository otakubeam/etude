#include <vm/interpreter.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: retval", "[vm]") {
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

TEST_CASE("vm: push and pop", "[vm]") {
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

TEST_CASE("vm: JUMP_IF_FALSE instr", "[vm]") {
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
                  // if this is executed, the value is 100 returned
                  // otherwise it's 500
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

TEST_CASE("vm: add", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push 100
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push 100
              },
              vm::Instr{
                  .type = vm::InstrType::ADD,
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 100},                //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(&chunk) == 200);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: fn call", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push 100
              },
              vm::Instr{
                  .type = vm::InstrType::CALL_FN,
                  .arg1 = 1,  // chunk 1 ~ compiled_fn
                  .arg2 = 0,  // ip is 0
                  .arg3 = 0,
              },
              vm::Instr{
                  .type = vm::InstrType::FIN_CALL,
                  .arg1 = 1,  // pop one arg from the stack
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 100},                //
      },
  };

  vm::ExecutableChunk compiled_fn{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::FROM_STACK,
                  .arg1 = 0,  // push argument i
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push constant 1
              },
              vm::Instr{
                  .type = vm::InstrType::ADD,
              },
              vm::Instr{
                  .type = vm::InstrType::RET_FN,
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 1},                  //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone({chunk, compiled_fn}) ==
        101);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: function with if statements", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push true
              },
              vm::Instr{
                  .type = vm::InstrType::CALL_FN,
                  .arg1 = 1,  // chunk 1 ~ compiled_fn
                  .arg2 = 0,  // ip is 0
                  .arg3 = 0,
              },
              vm::Instr{
                  .type = vm::InstrType::FIN_CALL,
                  .arg1 = 1,  // pop one arg from the stack
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Bool,  //
                                 .as_bool = true},               //
      },
  };

  vm::ExecutableChunk compiled_fn_invert_bool{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::FROM_STACK,
                  .arg1 = 0,  // push argument i
              },
              vm::Instr{
                  .type = vm::InstrType::JUMP_IF_FALSE,
                  .arg2 = 0,
                  .arg3 = 4,  // maybe skip next 2 instrs
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 1,  // push constant false
              },
              vm::Instr{
                  .type = vm::InstrType::RET_FN,
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push constant true
              },
              vm::Instr{
                  .type = vm::InstrType::RET_FN,
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Bool,  //
                                 .as_bool = true},               //
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Bool,  //
                                 .as_bool = false},              //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(
            {chunk, compiled_fn_invert_bool}) == (int)false);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: recursive function", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push 2
              },
              vm::Instr{
                  .type = vm::InstrType::CALL_FN,
                  .arg1 = 1,  // chunk 1 ~ compiled_fn
                  .arg2 = 0,  // ip is 0
                  .arg3 = 0,
              },
              vm::Instr{
                  .type = vm::InstrType::FIN_CALL,
                  .arg1 = 1,  // pop one arg from the stack
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 3},                  //
      },
  };

  vm::ExecutableChunk compiled_fn_sum{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::FROM_STACK,
                  .arg1 = 0,  // push argument i
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push constant 0
              },
              vm::Instr{
                  .type = vm::InstrType::CMP_EQ,
              },
              vm::Instr{
                  .type = vm::InstrType::JUMP_IF_FALSE,
                  .arg2 = 0,
                  .arg3 = 6,  // maybe skip next 2 instrs
              },

              // fini:
              // ----

              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push constant 0
              },
              vm::Instr{
                  .type = vm::InstrType::RET_FN,
              },

              // recur:
              // -----

              // 1) Place (i-1) as argument
              vm::Instr{
                  .type = vm::InstrType::FROM_STACK,
                  .arg1 = 0,  // push argument i
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 1,  // push constant -1
              },
              vm::Instr{
                  .type = vm::InstrType::ADD,
              },

              // 2) Call the function
              vm::Instr{
                  .type = vm::InstrType::CALL_FN,
                  .arg1 = 1,  // chunk 1 ~ compiled_fn
                  .arg2 = 0,  // ip is 0
                  .arg3 = 0,
              },
              vm::Instr{
                  .type = vm::InstrType::FIN_CALL,
                  .arg1 = 1,  // pop one arg from the stack
              },

              // 3) Add f(i-1) and i
              vm::Instr{
                  .type = vm::InstrType::FROM_STACK,
                  .arg1 = 0,  // push argument i
              },
              vm::Instr{
                  .type = vm::InstrType::ADD,
              },

              vm::Instr{
                  .type = vm::InstrType::RET_FN,
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 0},                  //
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = -1},                 //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(
            {chunk, compiled_fn_sum}) == 6);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm: using locals", "[vm]") {
  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push 2
              },
              vm::Instr{
                  .type = vm::InstrType::CALL_FN,
                  .arg1 = 1,  // chunk 1 ~ compiled_fn
                  .arg2 = 0,  // ip is 0
                  .arg3 = 0,
              },
              vm::Instr{
                  .type = vm::InstrType::FIN_CALL,
                  .arg1 = 1,  // pop one arg from the stack
              },
          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 3},                  //
      },
  };

  vm::ExecutableChunk compiled_fn_sum{
      .instructions =
          {
              vm::Instr{
                  .type = vm::InstrType::FROM_STACK,
                  .arg1 = 0,  // push argument i
              },
              vm::Instr{
                  .type = vm::InstrType::PUSH_STACK,
                  .arg1 = 0,  // push constant 0
              },
              vm::Instr{
                  .type = vm::InstrType::CMP_EQ,
              },

          },
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 0},                  //
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = -1},                 //
      },
  };

  CHECK(vm::BytecodeInterpreter::InterpretStandalone(
            {chunk, compiled_fn_sum}) == 6);
}

//////////////////////////////////////////////////////////////////////
