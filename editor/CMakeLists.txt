add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(veditor
  PUBLIC
    vulture
    fennecs
    fmt
    glad
    glfw
    glm
    stb_image
    imgui
  )