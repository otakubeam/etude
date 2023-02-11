add_executable(etc etc_cli.cpp)

target_link_libraries(etc PUBLIC compiler)

set_target_properties(etc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_compile_options(etc PRIVATE -fsanitize=undefined)
target_link_options(etc PRIVATE -fsanitize=undefined)