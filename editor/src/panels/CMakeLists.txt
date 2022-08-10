set(PANELS_INCLUDE_DIR ${EDITOR_DIR}/include/panels)
set(PANELS_SOURCE_DIR ${EDITOR_DIR}/src/panels)

target_sources(vulture-editor
  PUBLIC
    ${PANELS_INCLUDE_DIR}/entities_panel.hpp
    ${PANELS_INCLUDE_DIR}/inspector_panel.hpp
    ${PANELS_INCLUDE_DIR}/preview_panel.hpp

  PRIVATE
    ${PANELS_SOURCE_DIR}/entities_panel.cpp
    ${PANELS_SOURCE_DIR}/inspector_panel.cpp
    ${PANELS_SOURCE_DIR}/preview_panel.cpp
  )
