set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "app")

add_executable(repl repl.cpp)
target_link_libraries(repl PUBLIC compiler)
