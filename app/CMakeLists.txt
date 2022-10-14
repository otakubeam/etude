add_executable(repl repl.cpp)
target_link_libraries(repl PUBLIC compiler)
target_link_libraries(repl PUBLIC ubsan)
