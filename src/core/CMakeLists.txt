set(CORE_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/core)
set(CORE_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/core)

target_sources(engine
  PUBLIC
    ${CORE_INCLUDE_DIR}/core.hpp
    ${CORE_INCLUDE_DIR}/time.hpp
    ${CORE_INCLUDE_DIR}/logger.hpp
  PRIVATE
    ${CORE_SOURCE_DIR}/core.cpp
    ${CORE_SOURCE_DIR}/time.cpp
    ${CORE_SOURCE_DIR}/logger.cpp
  )