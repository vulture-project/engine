set(INPUT_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/platform)
set(INPUT_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/platform)

target_sources(engine
  PUBLIC
    ${INPUT_INCLUDE_DIR}/window.hpp
  PRIVATE
    ${INPUT_SOURCE_DIR}/window.cpp
  )