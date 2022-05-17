set(SANDBOX_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/scene)
set(SANDBOX_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/scene)

target_sources(engine
  PUBLIC
    ${SANDBOX_INCLUDE_DIR}/scene.hpp
  PRIVATE
    ${SANDBOX_SOURCE_DIR}/scene.cpp
  )