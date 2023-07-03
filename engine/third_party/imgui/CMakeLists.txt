add_library(imgui)

file(GLOB_RECURSE IMGUI_INCLUDE *.hpp *.h)
file(GLOB_RECURSE IMGUI_SOURCE *.cpp *.c)

target_include_directories(imgui
  PUBLIC
    ./include/
  )

target_sources(imgui
  PUBLIC
    ${IMGUI_INCLUDE}
  PRIVATE
    ${IMGUI_SOURCE}
  )

