set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

message(STATUS "Generating tests")

get_filename_component(TESTS_PATH "." ABSOLUTE)
file(GLOB_RECURSE TEST_SOURCES ${TESTS_PATH}/*.cpp)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE compiler)
target_link_libraries(tests PRIVATE Catch2::Catch2)
