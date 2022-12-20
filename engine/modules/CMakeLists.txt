add_library(fennecs)

file(GLOB_RECURSE FENNECS_INCLUDES *.hpp)
file(GLOB_RECURSE FENNECS_SOURCES *.cpp)

target_include_directories(fennecs
        PUBLIC
            .
        )

target_sources(fennecs
        PUBLIC
            ${FENNECS_INCLUDES}
        PRIVATE
            ${FENNECS_SOURCES}
        )
