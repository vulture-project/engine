file(GLOB_RECURSE ASSET_INCLUDE ${ENGINE_DIR}/include/asset/*.hpp)
file(GLOB_RECURSE ASSET_SOURCE ${ENGINE_DIR}/src/asset/*.cpp)

target_sources(vulture-engine
    PUBLIC
        ${ASSET_INCLUDE}
    PRIVATE
        ${ASSET_SOURCE}
    )