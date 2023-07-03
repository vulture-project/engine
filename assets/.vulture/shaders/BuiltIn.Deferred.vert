#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"

layout(location = 0) out vec2 texCoords;

void main() {
    texCoords = SCREEN_QUAD_POSITIONS[gl_VertexIndex];
    gl_Position = vec4(texCoords, 0.0, 1.0);
}