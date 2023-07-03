#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"

/* Push constants */
layout(push_constant) uniform ModelConstant
{
    mat4 uModel;
};

/* Attributes */
layout(location = 0) in vec3 aPositionMS;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormalMS;
layout(location = 3) in vec3 aTangentMS;
layout(location = 4) in vec3 aBitangentMS;

void main()
{
    vec4 positionWS = uModel * vec4(aPositionMS, 1.0);
    gl_Position = uProj * uView * positionWS;
}