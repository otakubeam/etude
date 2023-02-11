get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.hpp ${LIB_PATH}/*.ipp)

add_library(compiler STATIC ${LIB_CXX_SOURCES} ${LIB_HEADERS})
target_link_libraries(compiler PUBLIC fmt::fmt)

target_include_directories(compiler PUBLIC ${LIB_PATH})
