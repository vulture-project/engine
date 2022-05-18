set(AUDIO_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/audio)
set(AUDIO_SOURCE_DIR  ${ENGINE_SOURCE_DIR}/src/audio)

target_sources(engine
  PUBLIC
    ${AUDIO_INCLUDE_DIR}/AudioBuffer.h
    ${AUDIO_INCLUDE_DIR}/AudioContext.h
    ${AUDIO_INCLUDE_DIR}/AudioDevice.h
    ${AUDIO_INCLUDE_DIR}/AudioSource.h
    ${AUDIO_INCLUDE_DIR}/SoundUtils.h
    ${AUDIO_INCLUDE_DIR}/SourcePool.h
  PRIVATE
    ${AUDIO_SOURCE_DIR}/AudioBuffer.cpp
    ${AUDIO_SOURCE_DIR}/AudioContext.cpp
    ${AUDIO_SOURCE_DIR}/AudioDevice.cpp
    ${AUDIO_SOURCE_DIR}/AudioSource.cpp
    ${AUDIO_SOURCE_DIR}/SoundUtils.cpp
    ${AUDIO_SOURCE_DIR}/SourcePool.cpp
  )