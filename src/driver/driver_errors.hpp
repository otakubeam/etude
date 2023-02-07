#pragma once

#include <fmt/core.h>

#include <string>

struct DriverError : std::exception {
  std::string message;

  const char* what() const noexcept override {
    return message.c_str();
  }
};

struct NoStdlibError : DriverError {
  NoStdlibError(const std::string_view name) {
    message = fmt::format(
        "Could not open file '{}.et' in the current directory and "
        "'ETUDE_STDLIB' environment variable is not set. \n"
        "Please, refer to the documentation. \n",
        name);
  }
};
