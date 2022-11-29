#pragma once

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
#include <set>

class CompilationDriver {
 public:
  // 1. Searches in different places
  // 2. Opens the files and converts to ss
  std::stringstream OpenFile(std::string_view name) {
    std::ifstream file(std::string{name});

    // This is dumb! Lexer can take istream directly

    auto t = std::string(std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>());
    return std::stringstream{std::move(t)};
  }

  auto ParseOneModule(std::stringstream source)
      -> std::pair<Module, lex::Lexer> {
    auto l = lex::Lexer{source};
    Parser p{l};
    return {p.ParseModule(), std::move(l)};
  }

  auto ParseAllModules() {
    auto [main, lex] = ParseOneModule(OpenFile("main"));
    modules_.reserve(16);
    std::unordered_map<std::string_view, walk_status> visited;
    TopSort(&main, modules_, visited);
  }

  auto RegisterSymbols() {
    for (auto& m : modules_) {
      for (auto& exported_sym : m.exported_) {
        auto did_insert = module_of_.insert({exported_sym, &m}).second;

        // THINK: Is module import transitive?

        if (!did_insert) {
          // Be conservative for now
          throw "Conflicting exported symbols";
        }
      }
    }
  }

  enum walk_status {
    IN_PROGRESS,
    FINISHED,
    NOT_SEEN,
  };

  void TopSort(Module* node, std::vector<Module>& sort,
               std::unordered_map<std::string_view, walk_status>& visited) {
    for (auto& m : node->imports_) {
      if (visited.contains(m)) {
        if (visited[m] == IN_PROGRESS) {
          throw "Cycle in import higherarchy";
        }
        continue;
      }

      visited.insert({m, IN_PROGRESS});
      auto [mod, lex] = ParseOneModule(OpenFile(m));
      TopSort(&mod, sort, visited);
    }

    visited.insert_or_assign(node->GetName(), FINISHED);
    sort.push_back(*node);
  }

  // All its dependencies have already been completed
  void ProcessModule(Module* one) {
    one->BuildContext();
    one->MarkIntrinsics();
    one->InferTypes();
  }

  void Compile() {
    ParseAllModules();
    RegisterSymbols();

    // Those in the end have the least dependencies (see TopSort(...))
    for (size_t i = modules_.size() - 1; i >= 0; i -= 1) {
      ProcessModule(&modules_[i]);
    }

    auto inst_root = module_of_.at("main");
    auto main_sym = inst_root->GetExportedSymbol("main");
    inst_root->Compile(main_sym->GetFunctionDefinition());
  }

  Module* GetModuleOf(std::string_view symbol) {
    return module_of_.contains(symbol) ? module_of_[symbol] : nullptr;
  }

 private:
  // For each import map `symbol_name -> module`
  std::unordered_map<std::string_view, Module*> module_of_;

  std::vector<Module> modules_;
};
