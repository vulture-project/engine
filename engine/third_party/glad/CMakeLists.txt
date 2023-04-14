add_library(glad)

file(GLOB_RECURSE GLAD_INCLUDE *.hpp *.h)
file(GLOB_RECURSE GLAD_SOURCE *.cpp *.c)

target_include_directories(glad
  PUBLIC
    .
  )

target_sources(glad
  PUBLIC
    ${GLAD_INCLUDE}
  PRIVATE
    ${GLAD_SOURCE}
  )
