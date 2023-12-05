find_package(Catch2 QUIET)

if (NOT Catch2_FOUND)

  FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG master
  )

  FetchContent_MakeAvailable(Catch2)

endif (NOT Catch2_FOUND)

message(STATUS "Generating tests")

get_filename_component(TESTS_PATH "." ABSOLUTE)
file(GLOB_RECURSE TEST_SOURCES ${TESTS_PATH}/*.cpp)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE compiler)
target_link_libraries(tests PRIVATE Catch2::Catch2)

target_compile_options(tests PRIVATE -fsanitize=undefined)
target_link_options(tests PRIVATE -fsanitize=undefined)