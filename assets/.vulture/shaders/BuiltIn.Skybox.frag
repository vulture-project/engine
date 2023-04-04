#version 450

struct DirectionalLight
{
    // Specification
    vec3 color;
    float intensity;
    // Specification

    vec3 directionWS;
};

struct PointLight
{
    // Specification
    vec3 color;
    float intensity;
    float range;
    // Specification

    vec3 positionWS;
};

struct SpotLight
{
    // Specification
    vec3 color;
    float intensity;
    float range;
    float innerCone;  // TODO: (tralf-strues) precompute cosine value
    float outerCone;  // TODO: (tralf-strues) precompute cosine value
    // Specification

    vec3 positionWS;
    vec3 directionWS;
};

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

layout(set = 2, binding = 0) uniform LightEnvironment
{
  int uDirectionalLightsCount;
  int uPointLightsCount;
  int uSpotLightsCount;
};

layout(std140, set = 2, binding = 1) readonly buffer DirectionalLightsData
{
    DirectionalLight directionalLights[];
};

layout(std140, set = 2, binding = 2) readonly buffer PointLightsData
{
    PointLight pointLights[];
};

layout(std140, set = 2, binding = 3) readonly buffer SpotLightsData
{
    SpotLight spotLights[];
};

layout(set = 3, binding = 0) uniform samplerCube uSkybox;

layout(location = 0) in vec3 texCoords;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(uSkybox, texCoords);
}