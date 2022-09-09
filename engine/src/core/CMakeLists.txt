set(CORE_INCLUDE_DIR ${ENGINE_DIR}/include/core)
set(CORE_SOURCE_DIR ${ENGINE_DIR}/src/core)

target_sources(vulture-engine
  PUBLIC
    ${CORE_INCLUDE_DIR}/core.hpp
    ${CORE_INCLUDE_DIR}/logger.hpp
    ${CORE_INCLUDE_DIR}/resource_manager.hpp
    ${CORE_INCLUDE_DIR}/time.hpp
    ${CORE_INCLUDE_DIR}/uuid.hpp
  PRIVATE
    ${CORE_SOURCE_DIR}/core.cpp
    ${CORE_SOURCE_DIR}/logger.cpp
    ${CORE_SOURCE_DIR}/resource_manager.cpp
    ${CORE_SOURCE_DIR}/time.cpp
    ${CORE_SOURCE_DIR}/uuid.cpp
  )