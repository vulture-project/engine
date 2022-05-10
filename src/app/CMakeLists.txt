set(APP_INCLUDE_DIR ${ENGINE_SOURCE_DIR}/include/app)
set(APP_SOURCE_DIR ${ENGINE_SOURCE_DIR}/src/app)

target_sources(engine
  PUBLIC
    ${APP_INCLUDE_DIR}/app.hpp
  PRIVATE
    ${APP_SOURCE_DIR}/app.cpp
  )