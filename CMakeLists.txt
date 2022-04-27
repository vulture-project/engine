cmake_minimum_required(VERSION 3.7)

project(ENGINE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_executable(engine)

add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(engine
  PUBLIC
    fmt
    glad
    glfw
    glm
  )