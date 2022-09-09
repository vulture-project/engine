cmake_minimum_required(VERSION 3.13)

project(vulture-engine)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -DGLFW_INCLUDE_NONE -g -O2 -fsanitize=address")

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_INCLUDE_DIR ${ENGINE_DIR}/include)
set(ENGINE_SOURCE_DIR ${ENGINE_DIR}/src)

add_library(vulture-engine)

add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(vulture-engine
  PUBLIC
    fmt
    glad
    glfw
    glm
    OpenAL
    stb_image
    vorbisfile
    rapidjson
  )

target_link_options(OpenAL
  PUBLIC
  "LINKER:-lpthread"
  )
