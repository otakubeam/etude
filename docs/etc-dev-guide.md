
# ETC (Etude Compiler) developement guide

What do I want to talk about? Some non-obvious hiccups.
Perhaps explain why certain things are as they are.

## The stages of compilation

```
Parse && Build module tree 
-> Build symbol tables
-> Mark intrinsics
-> Infer types
  -> Collect constraints
  -> Resolve traits
-> Instantiate templates
-> Generate IR
```

## Compiler entry point

Compiler library is invoked by helper program in `/app`.
Run it with `etc` or `etc [name of your file]`. From there it beging processing
files.

In the first lines of every file you can write what other modules this file
uses. Then after parsing such include etc will try to locate these files either
in the `$ETUDE_STDLIB` path or in the current directory. 

## Then goes topological sort on modules

It then proceeds to recursively go in depth-first manner into these files, thus
building a topological sort on the modules.

As such (and as is seen in many languages), modules cannot cirularly depend on
each other. One module, however, can be split into multiple files. This is
currently unimplemented.

## Then goes calculating SCC on definition within one module

For each module starting from those that depend on nothing, etc computes the
strongly connected components of the dependency graph (i.e. who-calls-what).

Whis is required because circular dependencies need to be generalized together.

**Note:** Source code for this step lies in `/driver` directory.

## Symbol Table 

Ok, I am a bit ahead of myself. After parsing the file, the first thing
compiler does is building the symbol table. For each modules there is it's own
symbol table. And all these are connected through the driver. For example, if
we are unable to locate the definition of a symbol in the current file, we will
search it in all the imported files.

## Small step: mark intrinsics

This is somewhat of misdesign. But currently allows to call printf easily.
It converts print(...) funtion call to printf.

## Type inference

After building the symbol table we can proceed to collect constraints
using algorithm-w (in /src/types/infer/ directory).

These constraints are then sovled with resolver. In several passes.

TODO: detail what a pass is

## Template instantiation

After generalization step we can go and rewrite the tree. It starts with `main`
and functions marked as @test (unimplemented) and proceeds from there
recursively rewriting everything it calls.

## Then codegen

Serveral helping visitors: `pattern matching, address, gen_at`. Compound types
(structs, unions, etc..) are always placed on stack.

There is also measure for alignment. It calculated the real sizes.

Ok, I am tired for now. Will write the rest later...

## TODO: AST should be simple enough?
