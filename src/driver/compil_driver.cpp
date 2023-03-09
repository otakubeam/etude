#include <driver/compil_driver.hpp>

namespace cmd {

//////////////////////////////////////////////////////////////////////

std::stringstream Driver::OpenFile(std::string_view name) {
  auto module_name = std::string{name} + ".et";

  std::ifstream file(module_name);

  if (!file.is_open()) {
    if (auto path = std::getenv("ETUDE_STDLIB")) {
      std::filesystem::path stdlib{path};
      file = std::ifstream(stdlib / module_name);
    } else {
      throw NoStdlibError(name);
    }
  }

  if (!file.is_open()) {
    throw std::runtime_error{fmt::format("Could not open file {}", name)};
  }

  // This is dumb! Lexer can take istream directly

  auto t = std::string(std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>());
  return std::stringstream{std::move(t)};
}

//////////////////////////////////////////////////////////////////////

auto Driver::ParseOneModule(std::string_view name)
    -> std::pair<ModuleDeclaration*, lex::Lexer>  //
{
  auto source = OpenFile(name);

  auto lexer = lex::Lexer{name, source};
  auto mod = Parser{lexer}.ParseModule(name);

  return {std::move(mod), std::move(lexer)};
}

//////////////////////////////////////////////////////////////////////

void Driver::TopSort(ModuleDeclaration* node,                //
                     std::vector<ModuleDeclaration*>& sort,  //
                     Driver::VisitedMap& visited)            //
{
  for (auto& m : node->imports_) {
    if (visited.contains(m)) {
      if (visited[m] == IN_PROGRESS) {
        throw std::runtime_error{"Cycle in import higherarchy"};
      }
      continue;
    }

    visited.insert({m, IN_PROGRESS});
    auto [mod, lex] = ParseOneModule(m);
    TopSort(mod, sort, visited);
    lexers_.push_back(std::move(lex));
  }

  visited.insert_or_assign(node->GetName(), FINISHED);
  sort.push_back(node);
}

//////////////////////////////////////////////////////////////////////

void Driver::ParseAllModules() {
  auto [main, lex] = ParseOneModule(main_module_);
  modules_.reserve(16);
  std::unordered_map<std::string_view, walk_status> visited;
  TopSort(main, modules_, visited);
  lexers_.push_back(std::move(lex));
}

//////////////////////////////////////////////////////////////////////

// All its dependencies have already been completed
void Driver::ProcessModule(ModuleDeclaration* mod) {
  ast::elaboration::MarkIntrinsics mark;
  mod->Accept(&mark);

  mod->Accept(&ctx_builder_);
}

//////////////////////////////////////////////////////////////////////

void Driver::Compile() {
  ParseAllModules();

  for (auto& mod : modules_) {
    ProcessModule(mod);
  }

  global_context_.Print();

  std::exit(0);
}

//////////////////////////////////////////////////////////////////////

void Driver::SetTestBuild() {
  test_build = true;
}

//////////////////////////////////////////////////////////////////////

void Driver::SetMainModule(const char* mod) {
  main_module_ = mod;
}

//////////////////////////////////////////////////////////////////////

}  // namespace cmd
