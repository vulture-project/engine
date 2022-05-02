target_include_directories(engine
  PUBLIC
    ${ENGINE_SOURCE_DIR}/include
  )

target_sources(engine
  PUBLIC
    ${ENGINE_SOURCE_DIR}/src/core.cpp
    ${ENGINE_SOURCE_DIR}/src/main.cpp
  )

add_subdirectory(ECS)
add_subdirectory(platform)
add_subdirectory(renderer)
add_subdirectory(resource_loaders)