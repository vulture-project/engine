set(ECS_INCLUDE_DIR ${ENGINE_DIR}/include/ecs)
set(ECS_SOURCE_DIR ${ENGINE_DIR}/src/ecs)

target_sources(vulture-engine
  PUBLIC
    ${ECS_INCLUDE_DIR}/api.hpp
    ${ECS_INCLUDE_DIR}/component_holder.hpp
    ${ECS_INCLUDE_DIR}/entity_handle.hpp
    ${ECS_INCLUDE_DIR}/registry.hpp
    ${ECS_INCLUDE_DIR}/view.hpp
  PRIVATE
    ${ECS_SOURCE_DIR}/api.cpp
    ${ECS_SOURCE_DIR}/entity_handle.cpp
    ${ECS_SOURCE_DIR}/registry.cpp
  )
