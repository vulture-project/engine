target_include_directories(engine
  PUBLIC
    ${ENGINE_SOURCE_DIR}/include
  )

target_sources(engine
  PUBLIC
    ${ENGINE_SOURCE_DIR}/src/main.cpp
  )

add_subdirectory(core)
add_subdirectory(ecs)
add_subdirectory(platform)
add_subdirectory(renderer)
add_subdirectory(resource_loaders)
add_subdirectory(sandbox)
add_subdirectory(scene)
add_subdirectory(audio)