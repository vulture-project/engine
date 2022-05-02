set(RESOURCE_LOADERS_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/resource_loaders)
set(RESOURCE_LOADERS_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/resource_loaders)

target_sources(engine
  PUBLIC
    ${RESOURCE_LOADERS_INCLUDE_DIR}/parse_obj.hpp
  PRIVATE
    ${RESOURCE_LOADERS_SOURCE_DIR}/parse_obj.cpp
  )