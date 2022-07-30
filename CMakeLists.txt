cmake_minimum_required(VERSION 3.23)
project(interpreter)

# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -fno-omit-frame-pointer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# --------------------------------------------------------------------

find_package(fmt REQUIRED)
find_package(Catch2 2 REQUIRED)

# --------------------------------------------------------------------

add_subdirectory(src)

add_subdirectory(app)

add_subdirectory(tests)

# --------------------------------------------------------------------
