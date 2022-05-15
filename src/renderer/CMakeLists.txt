set(RENDERER_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/renderer)
set(RENDERER_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/renderer)

target_sources(engine
  PUBLIC
    ${RENDERER_INCLUDE_DIR}/buffer.hpp
    ${RENDERER_INCLUDE_DIR}/cube_map.hpp
    ${RENDERER_INCLUDE_DIR}/material.hpp
    ${RENDERER_INCLUDE_DIR}/renderer_api.hpp
    ${RENDERER_INCLUDE_DIR}/shader.hpp
    ${RENDERER_INCLUDE_DIR}/texture.hpp
  PRIVATE
    ${RENDERER_SOURCE_DIR}/buffer.cpp
    ${RENDERER_SOURCE_DIR}/cube_map.cpp
    ${RENDERER_SOURCE_DIR}/material.cpp
    ${RENDERER_SOURCE_DIR}/renderer_api.cpp
    ${RENDERER_SOURCE_DIR}/shader.cpp
    ${RENDERER_SOURCE_DIR}/texture.cpp
  )

add_subdirectory(2d)
add_subdirectory(3d)