cmake_minimum_required(VERSION 3.21)
project(vulture-engine)

include(cmake/build_options.cmake)
include(cmake/compile_options.cmake)

set(CMAKE_CXX_FLAGS "-DGLFW_INCLUDE_NONE")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../")

add_subdirectory(engine)
add_subdirectory(editor)