add_subdirectory(src)
add_subdirectory(third_party)

target_link_libraries(vulture
  PUBLIC
    assimp
    fennecs
    fmt
    glad
    glfw
    glm
    spirv-cross-core
    stb_image
    ${Vulkan_LIBRARY}
    ${Vulkan_LIBRARIES}
    VulkanMemoryAllocator
    yaml-cpp
  )
