add_library(imgui)

set(IMGUI_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/third_party/imgui/include)
set(IMGUI_SOURCE_DIR ${ENGINE_SOURCE_DIR}/third_party/imgui/src)

target_include_directories(imgui
  PUBLIC
    ${IMGUI_INCLUDE_DIR}
  )

target_sources(imgui
  PUBLIC
    ${IMGUI_INCLUDE_DIR}/imconfig.h
    ${IMGUI_INCLUDE_DIR}/imgui_internal.h
    ${IMGUI_INCLUDE_DIR}/imgui.h
    ${IMGUI_INCLUDE_DIR}/imstb_rectpack.h
    ${IMGUI_INCLUDE_DIR}/imstb_textedit.h
    ${IMGUI_INCLUDE_DIR}/imstb_truetype.h
  PRIVATE
    ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_SOURCE_DIR}/imgui.cpp
  )
