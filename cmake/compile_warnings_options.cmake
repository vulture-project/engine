set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
    add_compile_options("-Wpedantic")
endif()