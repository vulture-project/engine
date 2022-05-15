set(SANDBOX_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/src/sandbox)
set(SANDBOX_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/sandbox)

target_sources(engine
  PUBLIC
    ${SANDBOX_INCLUDE_DIR}/sandbox_app.hpp
  PRIVATE
    ${SANDBOX_SOURCE_DIR}/sandbox_app.cpp
  )