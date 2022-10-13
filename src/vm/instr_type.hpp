#pragma once

#include <vm/debug/debug_info.hpp>

#include <fmt/core.h>

#include <cstdlib>

namespace vm {

//////////////////////////////////////////////////////////////////////

enum class InstrType : u_int8_t {
  PUSH_VALUE,     // push $rt::Value
  PUSH_TRUE,      // push_true
  PUSH_FALSE,     // push_false
  PUSH_UNIT,      // push_unit
  PUSH_FP,        // push_fp (pushes the addr of fp on top of the stack)
  ADD_ADDR,       // add_addr $1 (add some small field offset to the addr)
  POP_STACK,      // pop
  RET_FN,         // ret
  CALL_FN,        // call $rt::InstrReference
  INDIRECT_CALL,  // inirect_call (pops $rt::Referece from stack)
  NATIVE_CALL,    // native_call $1 ($1 = offset in the table)
  FIN_CALL,       // fin_call $1
                  // (clean up $1 args from the stack and push eax)
  JUMP,           // jump $addr ($addr = relative displacement within a chunk)
  JUMP_IF_FALSE,  // jump_if_false $addr (relative displacement)
  GET_AT_FP,      // get_at_fp $addr (signed displacement)
  ADD,            // add (pops 2 and pushes result)
  SUBTRACT,       // subtract (pops 2 and pushes result)
  CMP_EQ,         // cmp_eq (pops 2 and pushes bool)
  CMP_LESS,       // cmp_eq (pops 2 and pushes bool)
  LOAD,           // load (pop address from the stack)
  STORE,          // store $1
                  // (pops address then pops value which is $1 words long)
  ALLOC,          // (pops the number of words to allocate)
};

//////////////////////////////////////////////////////////////////////

std::string FormatInstrType(InstrType type);

//////////////////////////////////////////////////////////////////////

}  // namespace vm
