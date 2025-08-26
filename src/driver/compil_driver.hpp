#pragma once

#include <driver/driver_errors.hpp>

#include <types/constraints/generate/algorithm_w.hpp>
#include <types/instantiate/instantiator.hpp>

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>
#include <ast/error_at_location.hpp>

#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

#include <qbe/ir_emitter.hpp>

#include <fmt/color.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <set>

class CompilationDriver {
 public:
  CompilationDriver(std::string_view main_mod = "main")
      : main_module_{main_mod} {
    // Types from previous run will reference already freed
    //    ast::scope::Symbols, we don't need them nor really
    //    should touch them or we'll get use after free.
    // Needed for multiple compilation driver invocations
    //    in one run, this approach is used by language server.
    types::ClearTypeStorage();
  }

  virtual ~CompilationDriver() = default;

  // 1. Searches in different places
  // 2. Opens the files and converts to ss
  // May be overriden by a derived class to search different places.
  //   Say, you have a filesystem layer that takes priority over
  //   files on disk (unsaved contents of files for language
  //   server) or package repository somewhere (tried to find
  //   another reason)...
  virtual lex::InputFile OpenFile(std::string_view name) {
    auto module_name = std::string{name} + ".et";
    auto abs_path = std::filesystem::absolute(module_name);

    std::ifstream file(module_name);

    if (!file.is_open()) {
      if (auto path = std::getenv("ETUDE_STDLIB")) {
        std::filesystem::path stdlib{path};
        file = std::ifstream(stdlib / module_name);
        abs_path = std::filesystem::absolute(stdlib / module_name);
      } else {
        throw NoStdlibError(name);
      }
    }

    if (!file.is_open()) {
      throw std::runtime_error{fmt::format("Could not open file {}", name)};
    }

    auto t = std::string(std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>());
    return lex::InputFile{std::stringstream{std::move(t)}, std::move(abs_path)};
  }

  auto ParseOneModule(std::string_view name) -> std::pair<std::unique_ptr<Module>, lex::Lexer> {
    auto source = OpenFile(name);
    auto lexer = lex::Lexer{std::move(source)};

    auto mod = Parser{lexer}.ParseModule();
    mod->SetName(name);

    return {std::move(mod), std::move(lexer)};
  }

  auto ParseAllModules() {
    auto [main, lex] = ParseOneModule(main_module_);
    std::unordered_map<std::string_view, walk_status> visited;
    TopSort(std::move(main), modules_, visited);
    lexers_.push_back(std::move(lex));
  }

  auto RegisterSymbols() {
    for (auto& m : modules_) {
      for (auto& exported_sym : m->exported_) {
        auto did_insert = module_of_.insert({exported_sym, m.get()}).second;

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

  void TopSort(std::unique_ptr<Module> node, std::vector<std::unique_ptr<Module>>& sort,
               std::unordered_map<std::string_view, walk_status>& visited) {
    for (auto& m : node->imports_) {
      if (visited.contains(m.GetName())) {
        if (visited[m.GetName()] == IN_PROGRESS) {
          throw ErrorAtLocation(m.location, "Cycle in import higherarchy");
        }
        continue;
      }

      visited.insert({m, IN_PROGRESS});
      try {
        auto [mod, lex] = ParseOneModule(m);
        TopSort(std::move(mod), sort, visited);
        lexers_.push_back(std::move(lex));
      } catch (const std::exception& exc) {
        throw ErrorAtLocation(m.location, std::string(exc.what()));
      }
    }

    visited.insert_or_assign(node->GetName(), FINISHED);
    sort.push_back(std::move(node));
  }

  // All its dependencies have already been completed
  void ProcessModule(Module* one) {
    one->BuildContext(this);
    one->MarkIntrinsics();
  }

  void SetTestBuild() {
    test_build = true;
  }

  void SetMainModule(const char* mod) {
    main_module_ = mod;
  }

  void Compile() {
    ParseAllModules();
    RegisterSymbols();

    // Those in the beginning have the least dependencies (see TopSort(...))
    for (size_t i = 0; i < modules_.size(); i += 1) {
      ProcessModule(modules_[i].get());
    }

    for (auto& m : modules_) {
      m->InferTypes(solver_);
    }

    if (test_build) {
      FMT_ASSERT(modules_.back()->GetName() == main_module_,
                 "Last module should be the main one");
      modules_.back()->Compile(nullptr);  // CompileTests
      return;
    }

    auto inst_root = module_of_.at("main");
    auto main_sym = inst_root->GetExportedSymbol("main");

    inst_root->Compile(main_sym->GetFunctionDefinition());
  }

  Module* GetModuleOf(std::string_view symbol) {
    return module_of_.contains(symbol) ? module_of_[symbol] : nullptr;
  }

 protected:
  std::string_view main_module_;

  // For each import map `symbol_name -> module`
  std::unordered_map<std::string_view, Module*> module_of_;

  std::vector<std::unique_ptr<Module>> modules_;
  std::vector<lex::Lexer> lexers_;

  bool test_build = false;

  types::constraints::ConstraintSolver solver_;
};
