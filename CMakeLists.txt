cmake_minimum_required(VERSION 3.23)
project(etude)

include(FetchContent)

# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -g -Og)

if(NOT "$ENV{BUILD_SANITIZERS}" STREQUAL "1")
    set(BUILD_SHARED_LIBS OFF) # So that it is portable across linuxes.
    if(WIN32 AND MSVC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    endif()
    # If not compiling for windows with msvc (mingw will work with -static)
    #   and not compiling for APPLE (necessary libraries should be static
    #   even without this, so we should depend on system libraries only,
    #   ldd step in CI will let us see that).
    # On apple it's not possible to build fully static.
    #   https://stackoverflow.com/questions/3801011/ld-library-not-found-for-lcrt0-o-on-osx-10-6-with-gcc-clan>
    #   https://stackoverflow.com/a/54890434
    if (NOT (WIN32 AND MSVC) AND NOT APPLE)
        add_compile_options(-static)
        add_link_options(-static)
    endif()
else()
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

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
