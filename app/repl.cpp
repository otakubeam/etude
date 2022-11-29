#include <driver/compil_driver.hpp>

#include <fmt/format.h>

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
  if (argc == 1) {
    fmt::print("Please provide a file as the first argument\n");
    exit(0);
  }

  CompilationDriver driver;
  driver.Compile();

  auto path = std::string{argv[1]};

}
