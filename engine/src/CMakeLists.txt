target_include_directories(vulture-engine
  PUBLIC
    ${ENGINE_DIR}/include/
  )

add_subdirectory(core)
add_subdirectory(ecs)
add_subdirectory(platform)
add_subdirectory(renderer)
add_subdirectory(resource_loaders)
add_subdirectory(scene)
add_subdirectory(audio)