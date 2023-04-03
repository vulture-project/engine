include("${CMAKE_HOME_DIRECTORY}/cmake/compile_options.cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../")
add_subdirectory(src)

target_link_libraries(veditor
  PUBLIC
    vulture
    fennecs
    fmt
    glfw
    glm
    stb_image
    imgui
  )