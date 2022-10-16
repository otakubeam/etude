add_executable(repl repl.cpp)

target_link_libraries(repl PUBLIC compiler)

target_link_libraries(repl PUBLIC ubsan)

set_target_properties(repl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/examples"
)

