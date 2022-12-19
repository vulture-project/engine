cmake_minimum_required(VERSION 3.13)

project(vulture-sandbox)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -DGLFW_INCLUDE_NONE -g -O2 -fsanitize=address")

add_executable(vulture-sandbox)

set(SANDBOX_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${SANDBOX_DIR}/../engine engine)
add_subdirectory(src)

target_link_libraries(vulture-sandbox
  PUBLIC
    vulture-engine
    fmt
    glad
    glfw
    glm
    # OpenAL
    stb_image
    # vorbisfile
  )