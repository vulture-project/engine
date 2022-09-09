set(SANDBOX_INCLUDE_DIR ${ENGINE_DIR}/include/scene)
set(SANDBOX_SOURCE_DIR ${ENGINE_DIR}/src/scene)

target_sources(vulture-engine
  PUBLIC
    ${SANDBOX_INCLUDE_DIR}/scene.hpp
  PRIVATE
    ${SANDBOX_SOURCE_DIR}/scene.cpp
  )