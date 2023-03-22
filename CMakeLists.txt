cmake_minimum_required(VERSION 3.21)
project(vulture-engine)

include(cmake/build_options.cmake)

If(APPLE)
  message("Its apple")
endif()

set(CMAKE_CXX_FLAGS "-DGLFW_INCLUDE_NONE")

add_subdirectory(engine)
# add_subdirectory(editor)