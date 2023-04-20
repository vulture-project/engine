set(CMAKE_CXX_STANDARD 17)

if(BUILD_WITH_ASAN AND BUILD_WITH_TSAN)
    message(FATAL_ERROR "asan and tsan can not be enabled at the same time")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-DON_DEBUG")

    if(WIN32)
        add_compile_options("-Od")    
    else()
        add_compile_options("-g")
        add_compile_options("-O0")
    endif()

    if(BUILD_WITH_ASAN AND NOT WIN32)
        message("-- Address sanitizer enabled")
        add_compile_options("-fsanitize=address")
        add_link_options("-fsanitize=address")
        # add_compile_options("-fsanitize=undefined")
        # add_link_options("-fsanitize=undefined")
    endif()
    
    if(BUILD_WITH_TSAN AND NOT WIN32)
        message("-- Thread sanitizer enabled")
        add_compile_options("-fsanitize=thread")
        add_link_options("-fsanitize=thread")
    endif()
else()
    add_compile_options("-O2")
    
    string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()