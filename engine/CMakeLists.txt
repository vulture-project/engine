add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(vulture
  PUBLIC
    fennecs
    fmt
    glad
    glfw
    glm
    stb_image
  )
