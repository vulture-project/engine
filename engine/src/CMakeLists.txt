add_library(vulture)

file(GLOB_RECURSE VULTURE_INCLUDE *.hpp *.h)
file(GLOB_RECURSE VULTURE_SOURCE *.cpp *.c)

list(FILTER VULTURE_INCLUDE EXCLUDE REGEX "vulture/audio/*")
list(FILTER VULTURE_SOURCE EXCLUDE REGEX "vulture/audio/*")

target_include_directories(vulture
  PUBLIC
    .
  )

target_sources(vulture
  PUBLIC
    ${VULTURE_INCLUDE}
  PRIVATE
    ${VULTURE_SOURCE}
  )