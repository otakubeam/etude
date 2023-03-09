#pragma once

#include <driver/driver_errors.hpp>

#include <types/constraints/generate/algorithm_w.hpp>

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <parse/parser.hpp>

#include <fmt/color.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

namespace cmd {

// Compilation Driver
class Driver {
 public:
  void Compile();

  void SetTestBuild();
  void SetMainModule(const char* mod);

 private:
  // 1. Searches for the file
  //      a) in stdlib path,
  //      b) in the cwd.
  // 2. Opens the file and converts it to the stringstream
  std::stringstream OpenFile(std::string_view name);

  enum walk_status { IN_PROGRESS, FINISHED, NOT_SEEN };

  using VisitedMap = std::unordered_map<std::string_view, walk_status>;

  void TopSort(ModuleDeclaration* node,                //
               std::vector<ModuleDeclaration*>& sort,  //
               VisitedMap& visited);

  auto ParseOneModule(std::string_view name)
      -> std::pair<ModuleDeclaration*, lex::Lexer>;

  void ParseAllModules();  // -> TopSort starting from the main module

  void ProcessModule(ModuleDeclaration* mod);

 private:
  bool test_build = false;

  std::string_view main_module_ = "main";

  std::vector<ModuleDeclaration*> modules_;

  // Store the lexer state in order to report errors
  std::vector<lex::Lexer> lexers_;

  ast::scope::Context global_context_;
  ast::scope::ContextBuilder ctx_builder_{global_context_};

  types::constraints::ConstraintSolver solver_;
};

}  // namespace cmd
