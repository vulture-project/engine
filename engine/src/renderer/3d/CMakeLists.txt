set(RENDERER_3D_INCLUDE_DIR ${RENDERER_INCLUDE_DIR}/3d)
set(RENDERER_3D_SOURCE_DIR ${RENDERER_SOURCE_DIR}/3d)

target_sources(vulture-engine
  PUBLIC
    ${RENDERER_3D_INCLUDE_DIR}/camera.hpp
    ${RENDERER_3D_INCLUDE_DIR}/light.hpp
    ${RENDERER_3D_INCLUDE_DIR}/mesh.hpp
    ${RENDERER_3D_INCLUDE_DIR}/renderer3d.hpp
    ${RENDERER_3D_INCLUDE_DIR}/scene3d.hpp
  PRIVATE
    ${RENDERER_3D_SOURCE_DIR}/camera.cpp
    ${RENDERER_3D_SOURCE_DIR}/mesh.cpp
    ${RENDERER_3D_SOURCE_DIR}/renderer3d.cpp
    ${RENDERER_3D_SOURCE_DIR}/scene3d.cpp
  )