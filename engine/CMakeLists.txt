add_subdirectory(third_party)

include("${CMAKE_HOME_DIRECTORY}/cmake/compile_options.cmake")
add_subdirectory(src)

target_link_libraries(vulture
  PUBLIC
    assimp
    fennecs
    fmt
    glfw
    glm
    spirv-cross-core
    spirv-cross-glsl
    stb_image
    ${Vulkan_LIBRARY}
    ${Vulkan_LIBRARIES}
    VulkanMemoryAllocator
    yaml-cpp
  )
