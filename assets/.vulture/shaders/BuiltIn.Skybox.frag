#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"

layout(set = 2, binding = 0) uniform samplerCube uSkybox;

layout(location = 0) in vec3 texCoords;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(uSkybox, texCoords);
}