set(ECS_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/ECS)
set(ECS_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/ECS)

target_sources(engine
  PUBLIC
    ${ECS_INCLUDE_DIR}/Component.h
    ${ECS_INCLUDE_DIR}/ComponentFactory.h
    ${ECS_INCLUDE_DIR}/Entity.h
    ${ECS_INCLUDE_DIR}/EntityFactory.h
    ${ECS_INCLUDE_DIR}/TypeIdGenerator.h
  PRIVATE
    ${ECS_SOURCE_DIR}/Entity.cpp
  )