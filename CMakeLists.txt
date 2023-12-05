cmake_minimum_required(VERSION 3.23)
project(etude)

include(FetchContent)

# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -g -Og -fsanitize=undefined)

# --------------------------------------------------------------------

find_package(fmt QUIET)

if (NOT fmt_FOUND)

  FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
  )

  FetchContent_MakeAvailable(fmt)

endif (NOT fmt_FOUND)

# find_package(Catch2 2 REQUIRED)

# --------------------------------------------------------------------

add_subdirectory(src)

add_subdirectory(app)

# add_subdirectory(tests)

# --------------------------------------------------------------------
