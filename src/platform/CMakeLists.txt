set(PLATFORM_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/platform)
set(PLATFORM_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/platform)

target_sources(engine
  PUBLIC
    ${PLATFORM_INCLUDE_DIR}/window.hpp
    ${PLATFORM_INCLUDE_DIR}/event.hpp
  PRIVATE
    ${PLATFORM_SOURCE_DIR}/window.cpp
    ${PLATFORM_SOURCE_DIR}/event.cpp
  )

add_subdirectory(opengl)