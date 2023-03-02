target_include_directories(vulture-engine
  PUBLIC
    ${ENGINE_DIR}/include/
  )

add_subdirectory(asset)
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(renderer)
add_subdirectory(scene)