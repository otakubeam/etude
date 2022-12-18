#pragma once

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <types/instantiate/instantiator.hpp>
#include <types/check/algorithm_w.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <qbe/ir_emitter.hpp>

#include <fmt/color.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <set>

class CompilationDriver {
 public:
  CompilationDriver(std::string_view main_mod = "main")
      : main_module_{main_mod} {
  }

  // 1. Searches in different places
  // 2. Opens the files and converts to ss
  std::stringstream OpenFile(std::string_view name) {
    auto module_name = std::string{name} + ".et";

    std::ifstream file(module_name);

    if (!file.is_open()) {
      auto path = std::getenv("ETUDE_STDLIB");
      fmt::print(stderr, "{}\n", std::string{path});
      std::filesystem::path stdlib{path};
      file = std::ifstream(stdlib / module_name);
    }

    if (!file.is_open()) {
      throw std::runtime_error{fmt::format("Could not open file {}", name)};
    }

    // This is dumb! Lexer can take istream directly

    auto t = std::string(std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>());
    return std::stringstream{std::move(t)};
  }

  auto ParseOneModule(std::string_view name) -> std::pair<Module, lex::Lexer> {
    auto source = OpenFile(name);
    auto l = lex::Lexer{source};
    auto mod = Parser{l}.ParseModule();
    mod.SetName(name);
    return {std::move(mod), std::move(l)};
  }

  auto ParseAllModules() {
    auto [main, lex] = ParseOneModule(main_module_);
    modules_.reserve(16);
    std::unordered_map<std::string_view, walk_status> visited;
    TopSort(&main, modules_, visited);
    lexers_.push_back(std::move(lex));
  }

  auto RegisterSymbols() {
    for (auto& m : modules_) {
      for (auto& exported_sym : m.exported_) {
        auto did_insert = module_of_.insert({exported_sym, &m}).second;

        // THINK: Is module import transitive?

        if (!did_insert) {
          // Be conservative for now
          throw std::runtime_error{
              fmt::format("Conflicting exported symbols {}", exported_sym)};
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
          throw std::runtime_error{"Cycle in import higherarchy"};
        }
        continue;
      }

      visited.insert({m, IN_PROGRESS});
      auto [mod, lex] = ParseOneModule(m);
      TopSort(&mod, sort, visited);
      lexers_.push_back(std::move(lex));
    }

    visited.insert_or_assign(node->GetName(), FINISHED);
    sort.push_back(*node);
  }

  // All its dependencies have already been completed
  void ProcessModule(Module* one) {
    one->BuildContext(this);
    one->MarkIntrinsics();
  }

  void Compile() {
    ParseAllModules();
    RegisterSymbols();

    // Those in the end have the least dependencies (see TopSort(...))
    // for (int i = modules_.size() - 1; i >= 0; i -= 1) {
    for (size_t i = 0; i < modules_.size(); i += 1) {
      ProcessModule(&modules_[i]);
    }

    for (auto& m : modules_) {
      m.InferTypes();
    }

    auto inst_root = module_of_.at("main");
    auto main_sym = inst_root->GetExportedSymbol("main");
    inst_root->Compile(main_sym->GetFunctionDefinition());
  }

  Module* GetModuleOf(std::string_view symbol) {
    return module_of_.contains(symbol) ? module_of_[symbol] : nullptr;
  }

 private:
  std::string_view main_module_;

  // For each import map `symbol_name -> module`
  std::unordered_map<std::string_view, Module*> module_of_;

  std::vector<Module> modules_;
  std::vector<lex::Lexer> lexers_;
};
