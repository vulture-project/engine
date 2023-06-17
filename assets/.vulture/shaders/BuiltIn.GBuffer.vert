#version 450

#include "include/BuiltIn.Common.glsl"
#include "include/BuiltIn.FrameData.glsl"
#include "include/BuiltIn.ViewData.glsl"
#include "include/BuiltIn.SceneData.glsl"

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

layout(location = 0) out vec3 positionWS;
layout(location = 1) out vec2 texCoords;
layout(location = 2) out mat3 TBN;

void main()
{
    positionWS = (uModel * vec4(aPositionMS, 1.0)).xyz;
    texCoords  = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    TBN = normalMatrix * mat3(aTangentMS.xyz, aBitangentMS, aNormalMS.xyz);

    gl_Position = uProj * uView * vec4(positionWS, 1.0);
}