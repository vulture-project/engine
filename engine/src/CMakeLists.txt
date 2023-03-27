set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../")

add_executable(vulture)

file(GLOB_RECURSE VULTURE_INCLUDE *.hpp *.h)
file(GLOB_RECURSE VULTURE_SOURCE *.cpp *.c)

list(FILTER VULTURE_INCLUDE EXCLUDE REGEX "vulture/audio/*")
# list(FILTER VULTURE_INCLUDE EXCLUDE REGEX "vulture/asset/loaders/obj_loader.hpp")
list(FILTER VULTURE_SOURCE EXCLUDE REGEX "vulture/audio/*")
# list(FILTER VULTURE_SOURCE EXCLUDE REGEX "vulture/asset/loaders/obj_loader.cpp")

target_include_directories(vulture
  PUBLIC
    .
  )

target_sources(vulture
  PUBLIC
    ${VULTURE_INCLUDE}
  PRIVATE
    main.cpp
    ${VULTURE_SOURCE}
  )