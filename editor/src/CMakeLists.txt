add_executable(veditor)

file(GLOB_RECURSE VEDITOR_INCLUDE *.hpp *.h)
file(GLOB_RECURSE VEDITOR_SOURCE *.cpp *.c)

target_include_directories(veditor
  PUBLIC
    .
  )

target_sources(veditor
  PUBLIC
    ${VEDITOR_INCLUDE}
  PRIVATE
    ${VEDITOR_SOURCE}
  )