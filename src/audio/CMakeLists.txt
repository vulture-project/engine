set(AUDIO_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/audio)
set(AUDIO_SOURCE_DIR  ${ENGINE_SOURCE_DIR}/src/audio)

target_sources(engine
  PUBLIC
    ${AUDIO_INCLUDE_DIR}/api.hpp
    ${AUDIO_INCLUDE_DIR}/audio_buffer.hpp
    ${AUDIO_INCLUDE_DIR}/audio_context.hpp
    ${AUDIO_INCLUDE_DIR}/audio_device.hpp
    ${AUDIO_INCLUDE_DIR}/audio_listener.hpp
    ${AUDIO_INCLUDE_DIR}/audio_source.hpp
    ${AUDIO_INCLUDE_DIR}/audio_utils.hpp
    ${AUDIO_INCLUDE_DIR}/buffer_manager.hpp
    ${AUDIO_INCLUDE_DIR}/source_pool.hpp
  PRIVATE
    ${AUDIO_SOURCE_DIR}/api.cpp
    ${AUDIO_SOURCE_DIR}/audio_buffer.cpp
    ${AUDIO_SOURCE_DIR}/audio_context.cpp
    ${AUDIO_SOURCE_DIR}/audio_device.cpp
    ${AUDIO_SOURCE_DIR}/audio_listener.cpp
    ${AUDIO_SOURCE_DIR}/audio_source.cpp
    ${AUDIO_SOURCE_DIR}/audio_utils.cpp
    ${AUDIO_SOURCE_DIR}/buffer_manager.cpp
    ${AUDIO_SOURCE_DIR}/source_pool.cpp
  )