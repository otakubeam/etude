# Function calls

- In vm there should be direct and indirect calling mechanisms.

  - If a function is defined as with a keyword `fun` it makes sense to mark it
    as direct and compile the jump target right into the instruction.

  - However, if the variable being called has been declared using `var`
    (perhaps returned from a function or something), we need a way of knowing
    what chunk to call dynamically

    So, indirect calls.  
    Function-Objects are represented in vm-runtime simply as chunk numbers.

    - First place the chunk no. on the stack, then call `INDIRECT_CALL`
    
    - What if they don't have a chunk (intrinsics)?
      
      Distinguish that at the decoding time with a union.
