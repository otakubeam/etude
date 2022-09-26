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
  POP_STACK,      // pop
  RET_FN,         // ret
  CALL_FN,        // call $rt::InstrReference
  INDIRECT_CALL,  // inirect_call (pops $rt::Referece from stack)
  NATIVE_CALL,    // native_call $1 ($1 = offset in the table)
  FIN_CALL,       // fin_call $1
                  // (clean up $1 args from the stack and push eax)
  JUMP,           // jump $addr ($addr = relative displacement within a chunk)
  JUMP_IF_FALSE,  // jump_if_false $addr (relative displacement)
  ADD,            // add (pops 2 and pushes result)
  SUBTRACT,       // subtract (pops 2 and pushes result)
  CMP_EQ,         // cmp_eq (pops 2 and pushes bool)
  CMP_LESS,       // cmp_eq (pops 2 and pushes bool)
  GET_ARG,        // get_arg $1
                  // (get value at offset -3 - $1 from the current fp)
  GET_LOCAL,      // get_local $1
                  // (get value at offset $1 from the current fp)
  LOAD,           // load (pop address from the stack)
  STORE,          // store (pops address then pops value)
};

//////////////////////////////////////////////////////////////////////

std::string FormatInstrType(InstrType type);

//////////////////////////////////////////////////////////////////////

}  // namespace vm
