add_library(imgui)

set(IMGUI_INCLUDE_DIR ${EDITOR_DIR}/third_party/imgui/include)
set(IMGUI_SOURCE_DIR ${EDITOR_DIR}/third_party/imgui/src)

target_include_directories(imgui
  PUBLIC
    ${IMGUI_INCLUDE_DIR}
  )

target_sources(imgui
  PUBLIC
    ${IMGUI_INCLUDE_DIR}/imconfig.h
    ${IMGUI_INCLUDE_DIR}/imgui_impl_glfw.h
    ${IMGUI_INCLUDE_DIR}/imgui_impl_opengl3_loader.h
    ${IMGUI_INCLUDE_DIR}/imgui_impl_opengl3.h
    ${IMGUI_INCLUDE_DIR}/imgui_internal.h
    ${IMGUI_INCLUDE_DIR}/imgui.h
    ${IMGUI_INCLUDE_DIR}/imstb_rectpack.h
    ${IMGUI_INCLUDE_DIR}/imstb_textedit.h
    ${IMGUI_INCLUDE_DIR}/imstb_truetype.h
  PRIVATE
    ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_impl_opengl3.cpp
    ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_SOURCE_DIR}/imgui.cpp
  )

target_link_libraries(imgui
  PUBLIC
    glad
    glfw
  )