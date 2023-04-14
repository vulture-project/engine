layout(set = 2, binding = 0) uniform LightEnvironment
{
  int uDirectionalLightsCount;
  int uPointLightsCount;
  int uSpotLightsCount;
};

layout(set = 2, binding = 1) readonly buffer DirectionalLightsData
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