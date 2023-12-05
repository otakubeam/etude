cmake_minimum_required(VERSION 3.23)
project(etude)

include(FetchContent)

function(fetch_package pakcage_name git_link git_branch)
  find_package("${pakcage_name}" QUIET)

  if (NOT "${pakcage_name}_FOUND")

    FetchContent_Declare("${pakcage_name}"
      GIT_REPOSITORY "${git_link}"
      GIT_TAG "${git_branch}"
    )

    FetchContent_MakeAvailable("${pakcage_name}")

  endif (NOT "${pakcage_name}_FOUND")
endfunction()

# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -g -Og -fsanitize=undefined)

# --------------------------------------------------------------------

fetch_package(fmt https://github.com/fmtlib/fmt.git master)

# --------------------------------------------------------------------

add_subdirectory(src)

add_subdirectory(app)

# add_subdirectory(tests)

# --------------------------------------------------------------------
