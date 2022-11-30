#include <driver/compil_driver.hpp>

#include <fmt/format.h>

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
  if (argc > 2) {
    fmt::print("Too many files!\n");
    std::exit(0);
  }

  auto path = argc == 2 ? std::string{argv[1]} : "main";
  CompilationDriver{path}.Compile();
}
