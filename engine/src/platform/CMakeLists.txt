set(PLATFORM_INCLUDE_DIR ${ENGINE_DIR}/include/platform)
set(PLATFORM_SOURCE_DIR ${ENGINE_DIR}/src/platform)

target_sources(vulture-engine
  PUBLIC
    ${PLATFORM_INCLUDE_DIR}/window.hpp
    ${PLATFORM_INCLUDE_DIR}/input.hpp
  PRIVATE
    ${PLATFORM_SOURCE_DIR}/window.cpp
    ${PLATFORM_SOURCE_DIR}/input.cpp
  )

add_subdirectory(opengl)