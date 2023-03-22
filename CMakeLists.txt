cmake_minimum_required(VERSION 3.21)
project(vulture-engine)

include(cmake/build_options.cmake)

cmake_policy(SET CMP0077 NEW)
set(CMAKE_CXX_FLAGS "-DGLFW_INCLUDE_NONE")

add_subdirectory(engine)
# add_subdirectory(editor)