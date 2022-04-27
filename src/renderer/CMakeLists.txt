set(RENDERER_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/renderer)
set(RENDERER_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/renderer)

target_sources(engine
  PUBLIC
    ${RENDERER_INCLUDE_DIR}/buffer.hpp
    ${RENDERER_INCLUDE_DIR}/camera.hpp
    ${RENDERER_INCLUDE_DIR}/core.hpp
    ${RENDERER_INCLUDE_DIR}/opengl_buffer.hpp
    ${RENDERER_INCLUDE_DIR}/opengl_renderer_api.hpp
    ${RENDERER_INCLUDE_DIR}/opengl_shader.hpp
    ${RENDERER_INCLUDE_DIR}/opengl_texture.hpp
    ${RENDERER_INCLUDE_DIR}/renderer_api.hpp
    ${RENDERER_INCLUDE_DIR}/renderer3d.hpp
    ${RENDERER_INCLUDE_DIR}/resource_loader.hpp
    ${RENDERER_INCLUDE_DIR}/scene.hpp
    ${RENDERER_INCLUDE_DIR}/shader.hpp
    ${RENDERER_INCLUDE_DIR}/texture.hpp
  PRIVATE
    ${RENDERER_SOURCE_DIR}/buffer.cpp
    ${RENDERER_SOURCE_DIR}/camera.cpp
    ${RENDERER_SOURCE_DIR}/core.cpp
    ${RENDERER_SOURCE_DIR}/opengl_buffer.cpp
    ${RENDERER_SOURCE_DIR}/opengl_renderer_api.cpp
    ${RENDERER_SOURCE_DIR}/opengl_shader.cpp
    ${RENDERER_SOURCE_DIR}/opengl_texture.cpp
    ${RENDERER_SOURCE_DIR}/renderer_api.cpp
    ${RENDERER_SOURCE_DIR}/renderer3d.cpp
    ${RENDERER_SOURCE_DIR}/resource_loader.cpp
    ${RENDERER_SOURCE_DIR}/scene.cpp
    ${RENDERER_SOURCE_DIR}/shader.cpp
    ${RENDERER_SOURCE_DIR}/texture.cpp
  )