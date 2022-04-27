set(ECS_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/ECS)
set(ECS_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/ECS)

target_sources(engine
  PUBLIC
    ${ECS_INCLUDE_DIR}/component.h
    ${ECS_INCLUDE_DIR}/component_factory.h
    ${ECS_INCLUDE_DIR}/entity.h
    ${ECS_INCLUDE_DIR}/entity_factory.h
    ${ECS_INCLUDE_DIR}/type_id_generator.h
  PRIVATE
    ${ECS_SOURCE_DIR}/entity.cpp
  )