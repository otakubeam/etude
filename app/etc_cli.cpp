#include <driver/compil_driver.hpp>

#include <fmt/format.h>

#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

void ParseOptions(CompilationDriver& driver, int argc, char** argv) {
  auto opt = '\0';
  while ((opt = getopt(argc, argv, "tm:")) != -1) {
    switch (opt) {
      case 't':
        driver.SetTestBuild();
        break;
      case 'm':
        driver.SetMainModule(optarg);
        break;
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-m] module [-t] \n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char** argv) {
  CompilationDriver driver;

  ParseOptions(driver, argc, argv);

  try {
    driver.Compile();
  } catch (const ErrorAtLocation& error) {
    fmt::println(stderr, "{}: {}", error.where().Format(), error.what());
    return 1;
  }
  // Ошибки общие, без местоположения, обрабатываются как раньше. Такие остались
  //   в CompilationDriver, когда не удалось открыть файл или не нашлась
  //   стандартная библиотека.

  return 0;
}
