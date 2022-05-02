set(PLATFORM_OPENGL_INCLUDE_DIR ${PLATFORM_INCLUDE_DIR}/opengl)
set(PLATFORM_OPENGL_SOURCE_DIR ${PLATFORM_SOURCE_DIR}/opengl)

target_sources(engine
  PUBLIC
    ${PLATFORM_OPENGL_INCLUDE_DIR}/opengl_buffer.hpp
    ${PLATFORM_OPENGL_INCLUDE_DIR}/opengl_renderer_api.hpp
    ${PLATFORM_OPENGL_INCLUDE_DIR}/opengl_shader.hpp
    ${PLATFORM_OPENGL_INCLUDE_DIR}/opengl_texture.hpp
  PRIVATE
    ${PLATFORM_OPENGL_SOURCE_DIR}/opengl_buffer.cpp
    ${PLATFORM_OPENGL_SOURCE_DIR}/opengl_renderer_api.cpp
    ${PLATFORM_OPENGL_SOURCE_DIR}/opengl_shader.cpp
    ${PLATFORM_OPENGL_SOURCE_DIR}/opengl_texture.cpp
  )