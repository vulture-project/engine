#version 450

/* Descriptor Sets */
layout(set = 0, binding = 0) uniform FrameData
{
    float uTime;
};

layout(set = 1, binding = 0) uniform ViewData
{
    mat4 uView;
    mat4 uProj;
    vec3 uCameraWS;
};

/* Push constants */
layout(push_constant) uniform ModelConstant
{
    mat4 uModel;
};

/* Attributes */
layout(location = 0) in vec3 aPositionMS;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormalMS;
layout(location = 3) in vec4 aTangentMS; // tangent.w = +-1; it shows the handedness of TBN

layout(location = 0) out vec3 positionWS;
layout(location = 1) out vec2 texCoords;
layout(location = 2) out vec3 normalWS;
layout(location = 3) out vec4 tangentWS;

void main()
{
    positionWS = (uModel * vec4(aPositionMS, 1.0)).xyz;
    texCoords  = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    normalWS          = normalize(normalMatrix * aNormalMS);
    tangentWS         = vec4(normalize(normalMatrix * aTangentMS.xyz), aTangentMS.w);

    gl_Position = uProj * uView * vec4(positionWS, 1.0);
}