cmake_minimum_required(VERSION 3.7)

project(ENGINE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -DGLFW_INCLUDE_NONE -g -fsanitize=address -fno-optimize-sibling-calls")

add_executable(engine)

add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(engine
  PUBLIC
    fmt
    glad
    glfw
    glm
    stb_image
  )
