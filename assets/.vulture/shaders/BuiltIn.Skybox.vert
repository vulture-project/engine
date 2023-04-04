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

layout(location = 0) out vec3 texCoords;

void main()
{
    texCoords = aPositionMS;

    // Passing w as z so that the resulting z-coordinate (and consequently depth value) in NDC is always 1.0
    gl_Position = (uProj * uView * vec4(aPositionMS + uCameraWS, 1.0)).xyww;
}