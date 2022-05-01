add_library(stb_image)

set(STB_IMAGE_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/third_party/stb_image/include)
set(STB_IMAGE_SOURCE_DIR ${ENGINE_SOURCE_DIR}/third_party/stb_image/src)

target_include_directories(stb_image
  PUBLIC
    ${STB_IMAGE_INCLUDE_DIR}
  )

target_sources(stb_image
  PUBLIC
    ${STB_IMAGE_INCLUDE_DIR}/stb_image/stb_image.h
  PRIVATE
    ${STB_IMAGE_SOURCE_DIR}/stb_image.cpp
  )
