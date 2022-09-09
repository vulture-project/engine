add_library(glad)

set(GLAD_INCLUDE_DIR ${ENGINE_DIR}/third_party/glad/include)
set(GLAD_SOURCE_DIR ${ENGINE_DIR}/third_party/glad/src)

target_include_directories(glad
  PUBLIC
    ${GLAD_INCLUDE_DIR}
  )

target_sources(glad
  PUBLIC
    ${GLAD_INCLUDE_DIR}/glad/glad.h
    ${GLAD_INCLUDE_DIR}/KHR/khrplatform.h
  PRIVATE
    ${GLAD_SOURCE_DIR}/glad.c
  )
