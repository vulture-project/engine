set(EDITOR_INCLUDE_DIR ${EDITOR_DIR}/include/)
set(EDITOR_SOURCE_DIR ${EDITOR_DIR}/src/)

target_include_directories(vulture-editor
  PUBLIC
    ${EDITOR_INCLUDE_DIR}/
    ${EDITOR_DIR}/../engine/include
  )

target_sources(vulture-editor
  PUBLIC
    ${EDITOR_INCLUDE_DIR}/camera_movement.hpp
    ${EDITOR_INCLUDE_DIR}/editor_app.hpp

  PRIVATE
    ${EDITOR_SOURCE_DIR}/camera_movement.cpp
    ${EDITOR_SOURCE_DIR}/editor_app.cpp
    ${EDITOR_SOURCE_DIR}/main.cpp
  )

add_subdirectory(panels)