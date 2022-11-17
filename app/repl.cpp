#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <types/instantiate/instantiator.hpp>
#include <types/check/algorithm_w.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <vm/codegen/compiler.hpp>
#include <vm/debug/disassember.hpp>
#include <vm/debug/debugger.hpp>
#include <vm/elf_file.hpp>

#include <qbe/ir_emitter.hpp>

#include <fmt/color.h>

#include <fstream>
#include <string>

void RunPhony(vm::ElfFile& elf) {
  vm::debug::Disassembler d;
  d.Disassemble(elf);

  vm::debug::Debugger debugger;
  debugger.Load(elf);
  debugger.StepToTheEnd();
}

void RunSilent(vm::ElfFile& elf) {
  vm::BytecodeInterpreter interpreter;
  interpreter.Load(elf);
  interpreter.RunToTheEnd();
}

int main(int argc, char** argv) {
  if (argc == 1) {
    fmt::print("Please provide a file as the first argument\n");
    exit(0);
  }

  auto path = std::string{argv[1]};

  std::ifstream file(path);

  auto source =
      std::stringstream{std::string((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>())};

  auto l = lex::Lexer{source};
  Parser p{l};
  auto result = p.ParseUnit();

  ast::scope::Context global_context;
  ast::scope::ContextBuilder ctx_builder{global_context};

  for (auto r : result) {
    r->Accept(&ctx_builder);
  }

  ast::elaboration::MarkIntrinsics mark;
  for (auto& r : result) {
    r = mark.Eval(r)->as<Statement>();
  }

  types::check::AlgorithmW infer;

  for (auto r : result) {
    r->Accept(&infer);
  }

  FunDeclStatement* main = nullptr;

  for (auto r : result) {
    if (auto fun = r->as<FunDeclStatement>()) {
      fmt::print(stderr, "searching {}\n", fun->GetFunctionName());
      if (fun->GetFunctionName() == "main") {
        main = fun;
      }
    }
  }

  types::check::TemplateInstantiator inst(main);
  auto funs = inst.Flush();

  qbe::IrEmitter ir;

  if (!strcmp(argv[2], "--native")) {
    for (auto f : funs) {
      f->Accept(&ir);
    }
    return 0;
  }

  vm::codegen::Compiler compiler;
  vm::debug::Disassembler d;

  auto elf = vm::ElfFile{{}, {}, {}, {}};
  for (auto& d : funs) {
    elf += compiler.Compile(d);
  }

  d.Disassemble(elf);

  if (argc >= 3 && !strcmp(argv[2], "--debug")) {
    RunPhony(elf);
  } else {
    RunSilent(elf);
  }
}
