set(RESOURCE_LOADERS_INCLUDE_DIR ${ENGINE_DIR}/include/resource_loaders)
set(RESOURCE_LOADERS_SOURCE_DIR ${ENGINE_DIR}/src/resource_loaders)

target_sources(vulture-engine
  PUBLIC
    ${RESOURCE_LOADERS_INCLUDE_DIR}/parse_obj.hpp
  PRIVATE
    ${RESOURCE_LOADERS_SOURCE_DIR}/parse_obj.cpp
  )