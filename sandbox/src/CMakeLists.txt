set(SANDBOX_INCLUDE_DIR ${SANDBOX_DIR}/include/)
set(SANDBOX_SOURCE_DIR ${SANDBOX_DIR}/src/)

target_include_directories(vulture-sandbox
  PUBLIC
    ${SANDBOX_INCLUDE_DIR}/
    ${SANDBOX_DIR}/../engine/include
  )

target_sources(vulture-sandbox
  PUBLIC
    ${SANDBOX_INCLUDE_DIR}/sandbox_app.hpp
  PRIVATE
    ${SANDBOX_SOURCE_DIR}/main.cpp
    ${SANDBOX_SOURCE_DIR}/sandbox_app.cpp
  )