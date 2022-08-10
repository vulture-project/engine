cmake_minimum_required(VERSION 3.13)

project(ENGINE)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -DGLFW_INCLUDE_NONE -g -O2 -fsanitize=address")

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
    OpenAL
    vorbisfile
    rapidjson
  )

target_link_options(OpenAL
  PUBLIC
  "LINKER:-lpthread"
  )
