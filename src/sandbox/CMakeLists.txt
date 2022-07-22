set(SANDBOX_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/src/sandbox)
set(SANDBOX_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/sandbox)

target_sources(engine
  PUBLIC
    ${SANDBOX_INCLUDE_DIR}/sandbox_app.hpp
    # ${SANDBOX_INCLUDE_DIR}/audio_sandbox.hpp
  PRIVATE
    ${SANDBOX_SOURCE_DIR}/sandbox_app.cpp
    # ${SANDBOX_SOURCE_DIR}/audio_sandbox.cpp
  )