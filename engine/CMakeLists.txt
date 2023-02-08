set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_INCLUDE_DIR ${ENGINE_DIR}/include)
set(ENGINE_SOURCE_DIR ${ENGINE_DIR}/src)

add_library(vulture-engine)

add_subdirectory(modules)
add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(vulture-engine
  PUBLIC
    fmt
    glad
    glfw
    glm
    stb_image
    fennecs
  )
