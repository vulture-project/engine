add_library(stb_image)

file(GLOB_RECURSE STB_IMAGE_INCLUDE *.hpp *.h)
file(GLOB_RECURSE STB_IMAGE_SOURCE *.cpp *.c)

target_include_directories(stb_image
  PUBLIC
    .
  )

target_sources(stb_image
  PUBLIC
    ${STB_IMAGE_INCLUDE}
  PRIVATE
    ${STB_IMAGE_SOURCE}
  )

